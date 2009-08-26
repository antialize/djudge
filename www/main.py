from pyjamas.ui.RootPanel import RootPanel
#from pyjamas.ui.TextArea import TextArea
#from pyjamas.ui.Label import Label
#from pyjamas.ui.Button import Button
from pyjamas.ui.HTML import HTML
from pyjamas.ui.FlexTable import FlexTable
from pyjamas.JSONService import JSONProxy, JSONService
from pyjamas.ui import SimplePanel,  Button, ScrollPanel
from pyjamas.ui.VerticalPanel import VerticalPanel
from pyjamas.ui.HorizontalPanel import HorizontalPanel
from pyjamas.ui.DialogBox import DialogBox
from pyjamas import Window
from pyjamas.ui.TabPanel import TabPanel
from pyjamas.ui.Frame import Frame
from pyjamas.ui.ListBox import ListBox
from pyjamas.ui.Button import Button
from pyjamas.ui.FormPanel import FormPanel
from pyjamas.ui.FileUpload import FileUpload
from pyjamas.ui.TextArea import TextArea
from pyjamas.ui.Hidden import Hidden
from pyjamas.ui.TextBox import TextBox
from pyjamas.ui.Hyperlink import Hyperlink
from pyjamas.ui.PasswordTextBox import PasswordTextBox
from pyjamas.ui.CheckBox import CheckBox
from pyjamas.Cookies import getCookie, setCookie
from pyjamas.Timer import Timer
from datetime import datetime
import pyjamas.DOM 
import sha256.js

cookie=None

class Gateway(JSONProxy):
    def __init__(self):
        JSONProxy.__init__(self, "../rpc.py", ["login","logout","listProblems","listSubmissions","getSubmissionDetails", "listUsers","getUserDetails","updateUser","validateCookie","deleteUser"])
gw=Gateway()

def pwhash(pw):
    pw = sha256Hash("bK7GrfE2tA2tZR++9kMPlgq8NiNdzmTJFbYY2rjN1efOFAIgnuC1jgVOcFYR79UNo5CwKQ8375j2"+pw)
    for i in range(1000):
        pw = sha256Hash(pw)
    return pw

class RPCBase:
    def onRemoteError(self, code, message, request_info):
        Window.alert(message)

class RPCCall(RPCBase):
    def __init__(self, f):
        self.f = f
    def onRemoteResponse(self, response, request_info):
        return self.f(response, request_info)
    
class LoginDialgoBox(DialogBox,RPCBase):
    def __init__(self, app):
        DialogBox.__init__(self)
        self.app = app
        self.table=FlexTable()
        self.table.setText(0, 0, "Please enter username and password")
        self.table.getFlexCellFormatter().setColSpan(0, 0, 2)
        self.table.setText(1, 0, "Username")
        self.handle = TextBox()
        h = getCookie('handle')
        self.handle.setText(h)
        self.table.setWidget(1, 1, self.handle)
        self.table.setText(2, 0, "Password")
        self.pwd = PasswordTextBox()
        self.table.setWidget(2, 1, self.pwd)

        self.table.setHTML(3,0,"")
        self.table.getFlexCellFormatter().setColSpan(3, 0, 2)        
        h = HorizontalPanel()
        self.table.setWidget(4,0, h)
        self.table.getFlexCellFormatter().setColSpan(4, 0, 2)
        h.add(Button("Ok", getattr(self, "onOk")))
        h.add(Button("Cancel", getattr(self, "onClose")))
        h.setSpacing(4)
        self.setHTML("<b>Login</b>")
        self.setWidget(self.table)
        left = (Window.getClientWidth() - 200) / 2
        top = (Window.getClientHeight() - 100) / 2
        self.setPopupPosition(left,top)
    
    def onRemoteResponse(self, response, request_info):
        if response == "invalid_cookie":
            self.show()
        elif response == None or len(response) < 3:
            self.table.setHTML(3,0,"<b>Invalid username or password</b>")
        else:
            self.app.cookie = response[0]
            setCookie('cookie',response[0],1000*60*60*24)
            self.hide()
            self.app.logininit(response[1],response[2])
            
    def onClose(self,env):
        Window.getLocation().setHref("http://www.google.com")

    def onOk(self, env):
        global gw
        setCookie('handle',self.handle.getText(),24*60*60*1000)
        x = self.pwd.getText()
        x = pwhash(x)
        gw.login(self.handle.getText(), x, self)

    def login(self):
        global gw
        if not self.app.cookie:
            self.show()
        else:
            gw.validateCookie(self.app.cookie, self)
        

class EntityTab(RPCBase):
    def __init__(self,app):
        self.app = app

    def add(self, name):
        r = self.getRoot()
        i = self.app.tabs.getWidgetIndex(r)
        if i != -1:
            self.app.tabs.remove(r)
            if i >= self.app.tabs.getWidgetCount()-2: 
                i = -1
        if i == -1:
            self.app.tabs.add(r, name, True)
            self.app.tabs.selectTab( self.app.tabs.getWidgetCount() -1 )
        else:
            self.app.tabs.insert(r, name, True, i+1)
            self.app.tabs.selectTab(i)

    def rename(self, name):
        self.add(name)

    def remove(self):
        self.app.tabs.removeByKey(self.getRoot().key)

    def getRoot(self):
        return self.table

class UploadProblemTab(EntityTab):
    def __init__(self,app):
        EntityTab.__init__(self, app)
        self.form = FormPanel()
        self.form.setEncoding("multipart/form-data")
        self.form.setMethod("post")
        self.table = FlexTable()
        self.table.setText(0,0,"Problem archive")
        self.file = FileUpload()
        self.file.setName("file");
        self.table.setWidget(0,1,self.file)
        self.form.setWidget(self.table)
        self.button = Button("Submit",self)
        self.table.setWidget(1,1, self.button)
        self.msg = HTML("<b>Uploading</b>")
        self.msg.setVisible(False)
        self.cookie = Hidden()
        self.cookie.setName("cookie")
        self.table.setWidget(2,0, self.cookie)
        self.table.setWidget(2,1, self.msg)
        self.form.setWidget(self.table)
        self.form.setAction("../upload.py/problem")
        self.add("Upload new problem")

    def onSubmitComplete(self,event):
        self.msg.setVisible(False)

    def onSubmit(self,evt):
        self.msg.setVisible(True)
        
    def onClick(self,evt):
        if self.app.cookie == None:
            self.app.login()
        else:
            self.cookie.setValue(self.app.cookie)
            self.form.submit()
        
    def getRoot(self):
        return self.form


class UsersTab(RPCBase):
    def __init__(self,app):
        global gw
        self.app=app
        self.vpanel = VerticalPanel()
        self.vpanel.setWidth("100%")
        self.table = FlexTable()
        self.table.setHTML(0, 0, "<b>ID</b>")
        self.table.setHTML(0, 1, "<b>Handle</b>")
        self.table.setHTML(0, 2, "<b>Name</b>")
        self.vpanel.add(self.table)
        if self.app.admin:
            self.btn = Button("Create User",self.createUser)
            self.vpanel.add(self.btn)
        self.cnt = 1
        gw.listUsers(self)

    def reload(self):
        global gw
        gw.listUsers(self)

    def createUser(self,_):
        UserTab(self.app, -1)

    def userLambda(self,uid):
        return lambda: UserTab(self.app, uid)

    def getRoot(self):
        return self.vpanel

    def onRemoteResponse(self, response, request_info):
        while self.cnt > 1:
            self.cnt -= 1
            self.table.removeRow(self.cnt)

        for line in response:
            l=Hyperlink(line[0])
            l.addClickListener(self.userLambda(line[0]));
            self.table.setWidget(self.cnt, 0, l)
            l=Hyperlink(line[1])
            l.addClickListener(self.userLambda(line[0]));
            self.table.setWidget(self.cnt, 1, l)
            l=Hyperlink(line[2])
            l.addClickListener(self.userLambda(line[0]));
            self.table.setWidget(self.cnt, 2, l)
            self.cnt += 1       

class UserTab(EntityTab):
    def __init__(self,app, uid):
        global gw
        EntityTab.__init__(self, app)
        self.table = FlexTable()
        self.uid = uid
        self.load()

    def load(self):
        global gw
        if self.uid == -1: 
            self.setup(None)
        else:
            gw.getUserDetails(self.app.cookie, self.uid, self)

    def setup(self, x):
        edit = self.uid == self.app.uid or self.app.admin
        if not x:
            x = {'id': -1, 'handle': '', 'name': '' , 'admin': '', 'email': ''}
        self.uid = x['id']

        self.name = TextBox()
        self.name.setText(x['name'])
        self.email = TextBox()
        self.email.setText(x['email'])
        self.password = PasswordTextBox()
        self.passwordRepeat = PasswordTextBox()
        self.handle = TextBox()
        self.admin = CheckBox()
        self.admin.setChecked( x['admin'] )
        self.admin.setEnabled( self.app.admin )
        self.table.setText(0,0, "ID");
        self.table.setText(0,1, x['id'])
        self.table.setText(1,0, "Handle")
        if x['id'] == -1:
            self.table.setWidget(1,1, self.handle)
        else:
            self.table.setText(1,1, x['handle'])
        self.table.setText(2,0, "Name");
        if edit:
            self.table.setWidget(2,1, self.name)
        else:
            self.table.setText(2,1, x['name'])
        self.table.setText(3,0, "Email")
        if edit:
            self.table.setWidget(3,1, self.email)
        else:
            self.table.setText(3,1, x['email'])
        self.table.setText(4,0, "Admin");
        self.table.setWidget(4,1, self.admin)
        if edit:
            self.table.setText(5,0, "Password");
            self.table.setWidget(5,1, self.password)
            self.table.setText(6,0, "Password Repeat");
            self.table.setWidget(6,1, self.passwordRepeat)
                     
            self.saveBtn = Button("Save",self.save)
            self.table.setWidget(7,1, self.saveBtn)
        if self.app.admin:
            self.table.setWidget(8,1, Button("Remove", self.delete))

        if self.uid == -1:
            self.add('New user')
        else:
            self.add(x['handle'])

    def onUpdate(self, response, request_info):
        self.uid = response
        self.load()
        self.app.usersTab.reload()

    def save(self, _):
        global gw
        pwd=""
        if self.password.getText() != "" or self.passwordRepeat.getText() != "":
            if self.password.getText() != self.passwordRepeat.getText():
                Window.alert("Passwords differ");
                return
            pwd = pwhash(self.password.getText())
        gw.updateUser(self.app.cookie, self.uid, self.handle.getText(), self.name.getText(), pwd, self.admin.isChecked(), self.email.getText(), RPCCall(self.onUpdate))

    def onDelete(self, response, request_info):
        self.app.usersTab.reload()
        self.remove()

    def delete(self,_):
        global gw
        gw.deleteUser(self.app.cookie, self.uid, RPCCall(self.onDelete) )

    def onRemoteResponse(self, response, request_info):
        self.setup(response)
        
class ProblemTab(EntityTab):
    def __init__(self,app, pid):
        global gw
        EntityTab.__init__(self, app)
        self.table = FlexTable()
        self.table.setHTML(0, 0, "<b>Implement me</b>")
        self.add("Foo problem "+pid)

class SubmissionTab(EntityTab):
    def __init__(self,app, sid):
        global gw
        EntityTab.__init__(self, app)
        gw.getSubmissionDetails(app.cookie, sid, self)

    def trcode(self, c):
        if c == 0:
            return "Success"
        elif c == 1:
            return "Exit none-zero"
        elif c == 2:
            return "Presentation error"
        elif c == 3:
            return "Time limit exceeded"
        elif c == 4:
            return "Abnormal termination"
        elif c == 5:
            return "Runtime error"
        elif c == 6:
            return "Output limit exceeded"
        elif c == 7:
            return "Internal error"
        elif c == 8:
            return "Compilation time exceeded"
        elif c == 9:
            return "Compile error"
        elif c == 10:
            return "Entry does not exist"
        elif c == 11:
            return "Bad command"
        elif c == 12:
            return "Pending"
        elif c == 13:
            return "Error untaring archive"
        elif c == 14:
            return "Wrong output"
        elif c == 15:
            return "Invalid memory reference"
        elif c == 16:
            return "Memory limit exceeded"
        elif c == 17:
            return "Divide by zero"
        else:
            return "Unknown"

    def onRemoteResponse(self, response, request_info):
        self.table = FlexTable()
        self.table.setText(0,0, "Problem");
        self.table.setText(0,1, response['problem']);
        self.table.setText(1,0, "Code");
        self.table.setText(1,1, self.trcode(response['code'])+ " ("+response['code']+")");
        self.table.setText(2,0, "Message");
        x = TextArea()
        x.setText(response['message'])
        x.setWidth(600);
        x.setHeight(300);
        pyjamas.DOM.setAttribute(x.getElement(), 'readOnly', 'readonly')
        pyjamas.DOM.setAttribute(x.getElement(), 'readonly', 'readonly')
        self.table.setWidget(2,1,x );
        self.table.setText(3,0, "Source");
        x = TextArea()
        x.setText(response['source'])
        x.setWidth(600);
        x.setHeight(300);
        pyjamas.DOM.setAttribute(x.getElement(), 'readOnly',  'readonly')
        pyjamas.DOM.setAttribute(x.getElement(), 'readonly',  'readonly')
        self.table.setWidget(3,1, x);
        self.add("Submission "+response['id'])
    
class ProblemsTab(RPCBase):
    def __init__(self,app):
        global gw
        self.app=app
        self.vpanel = VerticalPanel()
        self.vpanel.setWidth("100%")
        self.table = FlexTable()
        self.table.setHTML(0, 0, "<b>ID</b>")
        self.table.setHTML(0, 1, "<b>Name</b>")
        self.table.setHTML(0, 2, "<b>Submitted By</b>")
        self.vpanel.add(self.table)
        if self.app.admin:
            self.btn = Button("Upload new problem",self.uploadProblem)
            self.vpanel.add(self.btn)
        self.cnt = 1
        gw.listProblems(self)

    def problemLambda(self, pid):
        return lambda: ProblemTab(self.app, pid)

    def userLambda(self,uid):
        return lambda: UserTab(self.app, uid)

    def uploadProblem(self,_=None):
        UploadProblemTab(self.app)
        
    def getRoot(self):
        return self.vpanel

    def onRemoteResponse(self, response, request_info):
        for line in response:
            l=Hyperlink(line[1])
            l.addClickListener(self.problemLambda(line[1]));
            self.table.setWidget(self.cnt, 0, l)
            l=Hyperlink(line[0])
            l.addClickListener(self.problemLambda(line[1]));
            self.table.setWidget(self.cnt, 1, l)
            l=Hyperlink(line[3])
            l.addClickListener(self.userLambda(line[2]));
            self.table.setWidget(self.cnt, 2, l)
            self.app.submitTab.problem.insertItem(line[0]+" ("+line[1]+")",line[1],-1)
            self.cnt += 1       
            
class StatusTab(RPCBase):
    def __init__(self,app):
        self.app=app
        self.vpanel = VerticalPanel()
        self.vpanel.setWidth("100%")
        self.table = FlexTable()
        self.vpanel.add(self.table)
        btn = Button('Update',self.update)
        self.vpanel.add(btn)
        self.id2row = {}
        self.table.setHTML(0, 0, "<b>Time</b>")
        self.table.setHTML(0, 1, "<b>Problem</b>")
        self.table.setHTML(0, 2, "<b>User</b>")
        self.table.setHTML(0, 3, "<b>Status</b>")
        self.cnt=1
        self.time=None
        #self.update()
        Timer(4000,self)
        
    def onTimer(self, x):
        #self.update()
        Timer(4000,self)

    def update(self,_=None):
        global gw
        gw.listSubmissions(self.time, self)
    
    def problemLambda(self, pid):
        return lambda: ProblemTab(self.app, pid)

    def userLambda(self,uid):
        return lambda: UserTab(self.app, uid)

    def submissionLambda(self,sid):
        return lambda: SubmissionTab(self.app, sid)

    def onRemoteResponse(self, response, request_info):
#0  (x.id,               
#1 x.problem.name, 
#2 x.problem_id, 
#3 x.submitter.handle, 
#4 x.submitter_id,  
#5 status(x.code),
#6 x.judgeTime,
#7 x.submitTime )
        if response:
            for line in response:
                if line[0] in self.id2row:
                    idx =self.cnt- self.id2row[line[0]]
                    #alert(self.id2row[line[0]] + " " + idx + " " + self.cnt);
                    #return
                else:
                    idx = 1
                    self.table.insertRow(1)
                    self.id2row[line[0]] = self.cnt
                    self.cnt += 1          
                #Hyperlink
                self.table.setText(idx, 0, line[7])
                pl=Hyperlink(line[1])
                pl.addClickListener(self.problemLambda(line[2]));
                self.table.setWidget(idx, 1, pl)
                ul=Hyperlink(line[3])
                ul.addClickListener(self.userLambda(line[4]) );
                self.table.setWidget(idx, 2, ul)
                sl=Hyperlink(line[5])
                sl.addClickListener(self.submissionLambda(line[0]))
                self.table.setWidget(idx, 3, sl)
                if line[6] != 'None': self.time = max(self.time, line[6], line[7])
                else: self.time = max(self.time, line[7])

    def getRoot(self):
        return self.vpanel

class SubmitTab:
    def __init__(self,app):
        self.app=app

        self.form = FormPanel()
        self.form.setEncoding("multipart/form-data")
        self.form.setMethod("post")
        
        self.msg = HTML("<b>Uploading</b>")
        self.table = FlexTable()
        self.table.setText(0,0, "Problem")
        self.table.setText(1,0, "Language")
        self.table.setText(2,0, "Program File")
        self.table.setText(3,0, "Program source")
        self.problem = ListBox()
        self.problem.insertItem("Detect",-1,-1)
        self.problem.setName("problem")
        self.table.setWidget(0,1,self.problem)
        
        self.lang = ListBox()
        self.lang.insertItem("Detect","",-1)
        self.lang.insertItem("C/C++","cc",-1)
        self.lang.insertItem("Java","Java",-1)
        self.lang.insertItem("Python","Python",-1)
        self.lang.setName("lang")
        self.table.setWidget(1,1,self.lang)
        self.cookie = Hidden()
        self.cookie.setName("cookie")
        self.table.setWidget(5,0,self.cookie)

        self.file = FileUpload()
        self.file.setName("file");
        self.table.setWidget(2,1,self.file)
        
        self.source = TextArea()
        self.source.setName("source")
        self.source.setWidth("600");
        self.source.setHeight("400");
        self.source.setText("""//$$problem: 1$$
//$$language: cc$$
#include <unistd.h>
#include <stdio.h>
int main() {
  int a,b;
  scanf("%d %d",&a,&b);
  printf("%d\\n",a+b);
  return 0;
}""")
        self.source.addChangeListener(self.onChange)
        self.table.setWidget(3,1,self.source)

        self.button = Button("Submit",self)
        self.table.setWidget(4,1, self.button)
        self.table.setWidget(5,1, self.msg)
        self.msg.setVisible(False)
        self.form.setWidget(self.table)
        self.form.setAction("../upload.py/submit")

        self.form.addFormHandler(self)

    def onChange(self, src):
        if self.source.getText():
            self.file = FileUpload()
            self.file.setName("file");
            self.table.setWidget(2,1,self.file)

    def onSubmitComplete(self,event):
        self.msg.setVisible(False)

    def onSubmit(self,evt):
        self.msg.setVisible(True)
        
    def onClick(self,evt):
        if self.app.cookie == None:
            self.app.login()
        else:
            self.cookie.setValue(self.app.cookie)
            self.form.submit()
        
    def getRoot(self):
        return self.form
        #return self.table;
    
tps={}
tpid=0
class MyTabPanel(TabPanel):
    def __init__(self):
        global tps
        global tpid
        tps[tpid] = self
        self.id = tpid
        tpid += 1
        self.k2w = {}
        self.key=0
        TabPanel.__init__(self)
        
    def add(self, tab, text, closable=False):
        t = text
        if closable:
            t += " <a href\=""about:black\" onClick=\"document.modules.main.main.tps.get(%d).removeByKey(%d); return false;\">[x]</a>"%(self.id,self.key)
        TabPanel.add(self, tab, t, closable)
        tab.key = self.key
        self.k2w[self.key]=tab
        self.key+=1

    def removeByKey(self, key):
        self.remove(self.k2w[key])

class App:
    def closeTab(self, x):
        Window.alert("close " + x);

    def logininit(self, uid, admin):
        self.uid = uid
        self.admin = admin
        self.submitTab = SubmitTab(self)
        self.problemsTab = ProblemsTab(self)
        self.statusTab = StatusTab(self)
        self.usersTab = UsersTab(self)
        self.tabs.add(self.problemsTab.getRoot(), "Problems")
        self.tabs.selectTab(0)
        self.tabs.add(self.submitTab.getRoot(), "Submit")
        self.tabs.add(self.statusTab.getRoot(), "Status")
        self.tabs.add(self.usersTab.getRoot(), "Users")
        self.vpanel.add(self.tabs)
        self.vpanel.add( Button("Logout", self.logout) )

    def __init__(self):
        global app
        app = self
        self.vpanel = VerticalPanel()
        self.vpanel.setWidth("100%")
        self.tabs = MyTabPanel()
        RootPanel().add(self.vpanel)
        self.tabs.setWidth("100%")
        self.cookie = getCookie('cookie')
        ld = LoginDialgoBox(self)
        ld.login()


    def showProblemDetails(self, pid):
        pass

    def showUserInfo(self,uid):
        pass

    def logout(self, _):
        global gw
        gw.logout(self.cookie)
        self.cookie = None
        setCookie('cookie','',24*60*60)
        Window.getLocation().reload()

def switchToStatusTab():
    global app
    app.tabs.selectTab(2)
    app.statusTab.update()

if __name__ == '__main__':
    app = App()

                           

