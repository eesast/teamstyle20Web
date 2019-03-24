import docker
import os, sys, shutil, datetime, json
import random, string
from django.shortcuts import render
from django.http import HttpResponse, JsonResponse
from backend.models import Battle, Team
from battle.models import Room, Queue

root_path=os.getcwd()
AI_path=root_path+'/media/Codes/robot/robot.so'  # NOTE:记得在服务器上修改
so_path = root_path+'/media/Codes/output' # 用户编译好后的文件夹
codes_path = root_path+'/media/Codes'     # 用户代码文件夹
data_path = root_path+'/media/data'
image_name = 'ts20:v1.06'                        # NOTE:记得修改
room_lim=5  # 对战房间数，从1标号 

chars = string.ascii_letters + string.digits

def generate_key(len=128):
    s=''
    while True :
        s=''.join([random.choice(chars) for _ in range(len)])
        if Room.objects.filter(key=s).exists()==False:
            break
    return s

def get_room():
    ''' 获得对战房间号 '''
    for i in range(1,room_lim+1):
        if Room.objects.filter(room_id=i).exists()==False:
            return i
    return -1

def debug(request):
    return HttpResponse(str(Room.objects.all())+'\n'+str(Queue.objects.all()))

def clear(request):
    Room.objects.all().delete()
    Queue.objects.all().delete()
    return HttpResponse("Clear!")

def run_battle():
    ''' 检查队列是否为空，不为空则进行一次对战 '''
    random.seed()
    room_id=get_room()
    mem_lim='1g'
    if room_id==-1 or Queue.objects.all().exists()==False:
        return
    battle = Queue.objects.all()[0]
    battle_id = battle.battle_id
    out_volume=root_path+'/media/data/%d'%battle_id # 卷路径
    in_volume='/MyVolume'

    temp = Battle.objects.get(id=battle_id)
    temp.status=2
    temp.start_time=datetime.datetime.now()
    temp.save()
    battle.delete()
    key=generate_key()
    Room.objects.create(room_id=room_id, battle_id=battle_id, key=key)

    client = docker.from_env()
    client.containers.run(image_name, command='bash /ts20/bin/run.sh %s'%key ,tty=True, stdin_open=True, remove=True, detach=True, network_mode='host', volumes={out_volume : {'bind': in_volume}})
    
def add_battle(request):
    ''' 用于添加对战 '''
    # TODO:改成POST?
    if request.method!='GET':
        return HttpResponse('Not GET!')
    # 读取request，检查合法性，并且存入Battle数据库
    team_engaged = request.GET.get('teams', None)
    robot_num = int(request.GET.get('AInum', None))
    initiator_name = request.GET.get('initiator_name',None)
    status = 1
    if team_engaged==None or robot_num==None or initiator_name==None :
        return HttpResponse('Lose parameters.')
    team_engaged = json.loads(team_engaged)
    for team_name in team_engaged:
        team = Team.objects.filter(teamname=team_name)
        if team.exists()==False:
            return HttpResponse('No team:%s'%team_name)
        if team[0].valid!=15:
            return HttpResponse('Team \"%s\" is invalid!')

    battle = Battle.objects.create(team_engaged=team_engaged, robot_num=robot_num, status=status, initiator_name=initiator_name)
    battle_id=battle.id # 对战ID
    path = root_path+'/media/data/%d'%battle_id ;
    os.makedirs(path)
    d = {}
    cnt = 0
    for team_name in team_engaged:
        team = Team.objects.filter(teamname=team_name)
        d[cnt]=team_name
        for j in range(4):
            shutil.copyfile(so_path+'/%d_%d.so'%(team.id,j),path+'/libAI_%d_%d.so'%(cnt,j))
        cnt+=1
    for i in range(robot_num):
        for j in range(4):
            shutil.copyfile(AI_path,path+'/libAI_%d_%d.so'%(cnt,j))
        cnt+=1
    id_map = json.dumps(d)
    battle.id_map=id_map
    battle.save()
    Queue.objects.create(battle_id=battle_id)

    room = get_room()
    run_battle()
    return HttpResponse('Added to queue. Battle ID:%d, Spare room: %d'%(battle_id,room))

def view_result(request):
    ''' 查询对战结果，传入battle_id，返回一个JSON '''
    if request.method!='GET':
        return HttpResponse('Not GET!')
    battle_id = request.GET.get('battle_id', default=-1)
    if battle_id==-1 :
        return HttpResponse('Wrong parameter')
    battle = Battle.objects.filter(id=battle_id)
    if battle.exists()==False :
        return HttpResponse('Not Found')
    battle = battle[0]
    # 冠军、排名、得分
    result = json.loads(battle.result)
    id_map = json.loads(battle.id_map)
    champion = id_map[result['team_out_order'][-1]] # rule ?
    # ?????? TODO：确认规则
    
    

def end_battle(request):
    ''' 对战结束后收到的请求，注意检查key，以及删除Room '''
    if request.method!='GET':
        return HttpResponse('Not GET!')
    key = request.GET.get('key', default='-1')
    if Room.objects.filter(key=key).exists()==False:
        return HttpResponse('Wrong key! %s'%key)
    room = Room.objects.filter(key=key)[0]
    battle_id = room.battle_id
    room.delete()
    battle = Battle.objects.get(id=battle_id)
    team_engaged = json.loads(battle.team_engaged)
    items = os.listdir(data_path+'/%s'%battle_id)
    json_name = ''
    for name in items:
        if name.endswith('.json'):
            json_name=name
    result=open(json_name).read()
    battle.result=result

    for team_name in team_engaged:
        team = Team.get(teamname=team_name)
        pass
    # TODO:结束房间并修改选手、对战数据，增加对战历史，处理json

    battle.save()
    return HttpResponse('End battle successfully!')

def compile(request):
    ''' 给定队伍编号team_id以及职业编号(0~3)ind，进行代码编译并放入/media/Codes/output '''
    out_volume = root_path+'/media/temp'+generate_key(10)
    in_volume = '/MyVolume'
    os.makedirs(out_volume)

    team_id = request.GET.get('team_id',None)
    ind = request.GET.get('ind',None)
    if team_id==None or ind==None:
        return HttpResponse('Wrong parameter! Please specify the team_id and ind.')
    origin_path = codes_path+'/%s_%s.cpp'%(team_id,ind)
    target_path = so_path+'/%s_%s.so'%(team_id,ind)
    if os.path.isfile(origin_path)==False:
        return HttpResponse('No corresponding .cpp file!')
    if os.path.exists(codes_path+'/output')==False:
        os.makedirs(codes_path+'/output')
    #if os.path.isfile(target_path):
    #    os.remove(target_path)
    # 不删除原本的.so文件，避免队伍恶意拒绝参赛
    shutil.copyfile(origin_path, out_volume+'/code.cpp')
    
    client = docker.from_env()
    client.containers.run(image_name, command='bash /ts20/bin/compile.sh' ,tty=True, stdin_open=True, remove=True, network_mode='host', volumes={out_volume : {'bind': in_volume}})
    team=Team.objects.get(id=team_id)
    status=''
    if os.path.isfile(out_volume+'/out.so')==True:
        status='Compile successfully.'
        team.valid|=(1<<ind)
    else:
        status='Compile Error.'
    team.save()
    information=open(out_volume+'/log').read()
    ret={}
    ret['status']=status
    ret['information']=information
    shutil.copyfile(out_volume+'/out.so',target_path)
    shutil.rmtree(out_volume)
    return JsonResponse(ret)
