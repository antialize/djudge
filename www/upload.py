import problemmanagment
import usermanagment

def submit(req):
    msg=""
    try: 
    #Todo assert logged in
    #source = req.form['source']
    #x=repr(source)
        source = ""
        fn = ""
        lang = ""
        problem = -1
        cookie = ""
        if 'source' in req.form: source=req.form['source']
        if 'problem' in req.form: problem=int(req.form['problem'])
        if 'lang' in req.form: lang=req.form['lang']
        if 'cookie' in req.form: cookie=req.form['cookie']
        if 'file' in req.form: 
            z=req.form['file']
            fn=z.filename
            z = z.file.read()
            if z: source = z
        uid = usermanagment.getUid(cookie)
        msg = problemmanagment.submit(uid,problem,lang,source,fn)
    except Exception as e:
        msg = str(e)

    req.content_type = "text/html"
    if msg:
        msg = msg.replace('\\','\\\\').replace('"','\\"')
        return "<html><body><script>alert(\"%s\")</script></body></html>"%msg
    else:
        return """<html><body><script>window.parent.document.modules.main.main.switchToStatusTab();</script></body></html>"""
