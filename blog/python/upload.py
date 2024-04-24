#!/usr/bin/env python
# -*- coding: utf-8 -*-

import cgi
import cgitb
import os

cgitb.enable()

form = cgi.FieldStorage()
method = os.environ['REQUEST_METHOD']

# while True:
# 	pass

print()

if method == "POST":
    fileitem = form['fileToUpload']
    if fileitem.filename:
        fn = os.path.basename(fileitem.filename)
        open('./blog/python/' + fn, 'wb').write(fileitem.file.read())
        print(f"<html><body><h2>file '{fn}' complete.</h2></body></html>")
    else:
        print("<html><body><h2>fail.</h2></body></html>")
else:
    print("""
    <html>
    <body>
    <form enctype="multipart/form-data" action="" method="post">
    <p>file: <input type="file" name="fileToUpload"></p>
    <p><input type="submit" value="upload"></p>
    </form>
    </body>
    </html>
    """)

