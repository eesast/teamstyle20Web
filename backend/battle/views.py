import docker
import os, sys, shutil
from django.shortcuts import render
from django.http import HttpResponse

# Create your views here.

def index(request):
    outVolumn=os.getcwd()+'/battle/data' # 卷路径
    inVolumn='/MyVolume'
    memLim='128m'

    n=0 # 总人数
    pathList=['battle/views.py']

    # os.removedirs(outVolumn)
    for i in range(n):
        if not os.path.isfile(pathList[i]):
            print("%s not exist!"%pathList[i])
            sys.exit(1)
        shutil.copyfile(pathList[i], outVolumn+'/%d'%i)

    client = docker.from_env()
    client.containers.run('ubuntu', command='touch /MyVolume/HelloWorld' ,tty=True, stdin_open=True, remove=True, detach=True, mem_limit=memLim, volumes={outVolumn : {'bind': inVolumn}})
    return HttpResponse('Run successfully!')
