#!/usr/bin/python
# -*- coding: UTF-8 -*-
from __future__ import absolute_import
from __future__ import with_statement
from __future__ import division
from __future__ import print_function

'''
Python code to rebuild the index, sice github pages doesn't provide
browsing links automatically
'''

import os,sys
import locale
from functools import cmp_to_key
import re

template = '''
<html>
<head>
</head>
<body>
<h3>Index of %(foldername)s</h3>
%(content)s
</body>
</html>
'''

# TODO: better way to handle flags
VERBOSE          = "--verbose"    in sys.argv
MAKE_PARENT_LINK = "--parentlink" in sys.argv

scriptfile = sys.argv[0]
if VERBOSE: 
    print('script at: ',scriptfile)


foldername = os.path.relpath(".","..")#os.getcwd()

def natural_key(string_):
    """
    See http://www.codinghorror.com/blog/archives/001018.html
    """
    return [int(s) if s.isdigit() else s for s in re.split(r'(\d+)', string_)]

# put directories before files
dirlist  = ''
filelist = ''

if MAKE_PARENT_LINK:
    dirlist+='<a href="../index.html">../</a><br/>'%globals()

for f in os.listdir('.'):
    # Ignore . files
    if f[0]=='.': 
        continue
    if f[-1]=='~': 
        continue
    if VERBOSE: print(f)
    if os.path.isdir('./'+f):
        if VERBOSE: 
            print('dir')
        os.chdir(f)
        cmd = '../'+scriptfile+' --parentlink'
        if VERBOSE: 
            print(cmd)
        os.system(cmd)
        os.chdir('../')
        dirlist+='<a href="./%(f)s/index.html">./%(f)s</a><br/>'%globals()
    elif os.path.os.path.isfile('./'+f):
        if f=="index.html": 
            continue
        if VERBOSE: 
            print('file')
        filelist+='<a href="./%(f)s">%(f)s</a><br/>'%globals()
    else:
        if VERBOSE: 
            print('unknown!?')

content = dirlist + filelist

with open("index.html", "w") as index:
    index.write(template%globals())

if VERBOSE: 
    print("done")
