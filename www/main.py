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
from pyjamas.ui.PasswordTextBox import PasswordTextBox
cookie=None

class Gateway(JSONProxy):
    def __init__(self):
        JSONProxy.__init__(self, "../rpc.py", ["login","logout","listProblems"])

gw=Gateway()

class LoginDialgoBox(DialogBox):
    def __init__(self, app):
        DialogBox.__init__(self)
        self.app = app
        self.table=FlexTable()
        self.table.setText(0, 0, "Please enter username and password")
        self.table.getFlexCellFormatter().setColSpan(0, 0, 2)
        self.table.setText(1, 0, "Username")
        self.handle = TextBox() 
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
        if(response == None):
            self.table.setHTML(3,0,"<b>Invalid username or password</b>")
        else:
            self.app.cookie = response
            Window.alert(self.app.cookie)
            self.app.loginButton.setText("Log out")
            self.hide()
            
    def onRemoteError(self, code, message, request_info):
        Window.alert(message)

    def onClose(self,env):
        self.hide()

    def onOk(self, env):
        global gw
        gw.login(self.handle.getText(), self.pwd.getText(), self)
        
    
class ProblemsTab:
    def __init__(self,app):
        global gw
        self.app=app
        self.table = FlexTable()
        self.table.setHTML(0, 0, "<b>ID</b>")
        self.table.setHTML(0, 1, "<b>Name</b>")
        self.table.setHTML(0, 2, "<b>Submitted By</b>")
        self.cnt = 1
        gw.listProblems(self)
        
    def getRoot(self):
        return self.table;

    def onRemoteResponse(self, response, request_info):
        for line in response:
            self.table.setText(self.cnt, 0, line[1])
            self.table.setText(self.cnt, 1, line[0])
            self.table.setText(self.cnt, 2, line[3])
            self.app.submitTab.problem.insertItem(line[0]+" ("+line[1]+")",line[1],-1)
            self.cnt += 1       
            
    def onRemoteError(self, code, message, request_info):
        Window.alert(message)

class StatusTab:
    def __init__(self,app):
        self.app=app
        self.hat = HTML("<b>GO AWAY</b>")

    def getRoot(self):
        return self.hat

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
        self.table.setWidget(3,1,self.source)

        self.button = Button("Submit",self)
        self.table.setWidget(4,1, self.button)
        self.table.setWidget(5,1, self.msg)
        self.msg.setVisible(False)
        self.form.setWidget(self.table)
        self.form.setAction("../upload.py/submit")

        self.form.addFormHandler(self)

    def onSubmitComplete(self,event):
        self.msg.setVisible(False)
        self.app.tabs.selectTab(2)

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
    

class App:
    def __init__(self):
        self.vpanel = VerticalPanel()
        self.vpanel.setWidth("100%")
        self.tabs = TabPanel()
        RootPanel().add(self.vpanel)
        
        self.tabs.setWidth("100%")
        self.submitTab = SubmitTab(self)
        self.problemsTab = ProblemsTab(self)
        self.statusTab = StatusTab(self)
        self.cookie = None

        self.tabs.add(self.problemsTab.getRoot(), "Problems")
        self.tabs.selectTab(0)
        self.tabs.add(self.submitTab.getRoot(), "Submit")
        self.tabs.add(self.statusTab.getRoot(), "Status")
        self.vpanel.add(self.tabs)
        
        self.loginButton = Button("Log in",self.swaplog)
        self.vpanel.add(self.loginButton)

    def swaplog(self,_):
        if self.cookie:
            self.logout()
        else:
            self.login()

    def login(self):
        ld = LoginDialgoBox(self)
        ld.show()

    def logout(self):
        global gw
        gw.logout(self.cookie)
        self.cookie = None
        self.loginButton.setText("Log in")
if __name__ == '__main__':
    app = App()

                           

