import docker
import os, sys, shutil
import random, string
from django.shortcuts import render
from django.http import HttpResponse

def generate_key(len=128):
    chars = string.ascii_letters + string.digits
    return ''.join([random.choice(chars) for _ in range(len)])

def getRoom():
    ''' 获得对战房间号 '''
    pass

def index(request):
    random.seed()
    roomLim=0 # 对战房间数，从1标号
    roomId=getRoom()
    outVolumn=os.getcwd()+'/battle/data/%d'%roomId # 卷路径
    inVolumn='/MyVolume'
    memLim='128m'

    n=0 # 总人数
    pathList=['battle/views.py']

    if(os.path.exists(outVolumn)):
        os.removedirs(outVolumn)
    os.mkdir(outVolumn)
    for i in range(n):
        if not os.path.isfile(pathList[i]):
            print("%s not exist!"%pathList[i])
            sys.exit(1)
        shutil.copyfile(pathList[i], outVolumn+'/%d'%i)

    client = docker.from_env()
    client.containers.run('ubuntu', command='touch /MyVolume/HelloWorld' ,tty=True, stdin_open=True, remove=True, detach=True, mem_limit=memLim, volumes={outVolumn : {'bind': inVolumn}})
    return HttpResponse('Run successfully!')
