import docker
import os, sys, shutil, datetime, json, jwt
import random, string
import numpy as np
from django.shortcuts import render
from django.http import HttpResponse, JsonResponse
from backend.models import Battle, Team
from backend import views
from battle.models import Room, Queue

root_path=os.getcwd()
AI_IND = 1000
is_final = True
time_limits = 65   #minutes   just for finals
AI_path=root_path+'/media/Codes/robot/robot.so'  # NOTE:记得在服务器上修改
so_path = root_path+'/media/Codes/output' # 用户编译好后的文件夹
codes_path = root_path+'/media/Codes'     # 用户代码文件夹
data_path = root_path+'/media/data'
image_name = 'ts20:v1.30'                        # NOTE:记得修改
room_lim=1  # 对战房间数，从1标号 

chars = string.ascii_letters + string.digits

def generate_key(len=128):
    s=''
    while True :
        s=''.join([random.choice(chars) for _ in range(len)])
        if Room.objects.filter(key=s).exists()==False:
            break
    return s

def softmax(arr):
    e_arr = np.exp(arr-np.max(arr))
    return e_arr/np.sum(e_arr)

def update(mmr, result, k=1000, mmr_zoom=0.01, result_zoom=0.01, beta=0.85):
    predict  = softmax(mmr*mmr_zoom)
    actual   = softmax(result*result_zoom)
    this_mmr = mmr + k*(actual-predict)
    return mmr*beta+(1-beta)*this_mmr

def get_room():
    ''' 获得对战房间号 '''
    for i in range(1,room_lim+1):
        if Room.objects.filter(room_id=i).exists()==False:
            return i
    return -1

def debug_view_queue(request):
    
    return HttpResponse(str(Room.objects.all())+'\n'+str(Queue.objects.all()))

def debug_view_team(request):
    team_id=request.GET.get('team_id',default=-1)
    if team_id==-1:
        return HttpResponse(str(Team.objects.all()))
    team_id=int(team_id)
    team = Team.objects.get(id=team_id)
    return HttpResponse(team.get_details())

def debug_view_battle(request):
    return HttpResponse(str(Battle.objects.all()))
    
def debug_clear_queue(request):
    ''' 清除Room和Queue '''
    Room.objects.all().delete()
    Queue.objects.all().delete()
    return HttpResponse("Clear Room and Queue!")

def debug_clear_team(request):
    ''' 清除Team '''
    Team.objects.all().delete()
    return HttpResponse("Clear Team!")

def debug_clear_battle(request):
    ''' 清除Battle '''
    Battle.objects.all().delete()
    return HttpResponse("Clear Battle!")

def debug_add_team(request):
    if request.method!='GET':
        return HttpResponse('Not GET!')
    teamname = request.GET.get('teamname', default='-1')
    if teamname=='-1' :
        return HttpResponse('Lose parameter')
    Team.objects.create(teamname=teamname)
    id=Team.objects.get(teamname=teamname).id
    return HttpResponse('Create successfully! teamname:%s, id:%d'%(teamname,id))

final_list = [30,37,39,40,42,45,50,51,52,54,58,60,64,68,70,71]

def start_finals():
# NOTE: final is ended
    return
#
    room_size=len(Room.objects.all())
    if(room_size!=0):
        now = datetime.datetime.utcnow()
        room=Room.objects.all()[0]
        battle_id=room.battle_id
        battle=Battle.objects.get(id=battle_id)
        t=battle.start_time
        t=t.replace(tzinfo=None)
        minu=(now-t).total_seconds()/60
        if minu > time_limits:
            battle.status=3
            battle.save()
            room.delete()
        else:
            return HttpResponse('Has been running for %d minutes; time_limits:%d'%(minu,time_limits))
    team_engaged = []
    for i in final_list:
        team_engaged.append(i)
    robot_num = 0
    initiator_id = 30
    status = 1
        
    battle = Battle.objects.create(team_engaged=json.dumps(team_engaged), robot_num=robot_num, status=status, initiator_id=initiator_id)
    battle_id=battle.id # 对战ID
    path = root_path+'/media/data/%d'%battle_id ;
    os.makedirs(path)
    d = {}
    cnt = 0
    for team_id in team_engaged:
        team = Team.objects.get(id=team_id)
        team_name = team.teamname
        d[cnt]=team_id
        for j in range(4):
            shutil.copyfile(so_path+'/%d_%d.so'%(team.id,j),path+'/libAI_%d_%d.so'%(cnt,j))
        cnt+=1
        if team_id!=initiator_id: # 更改对战历史
            tmp=json.loads(team.history_passive)
            tmp.append(battle_id)
            team.history_passive=json.dumps(tmp)
        else:
            tmp=json.loads(team.history_active)
            tmp.append(battle_id)
            team.history_active=json.dumps(tmp)
        team.save()
        
    for i in range(robot_num):
        #d[cnt]='__AI%d'%i # 与队伍命名不可冲突
        d[cnt]=AI_IND+cnt ;
        for j in range(4):
            shutil.copyfile(AI_path,path+'/libAI_%d_%d.so'%(cnt,j))
        cnt+=1
    id_map = json.dumps(d)
    battle.id_map=id_map
    battle.save()
    
    random.seed()
    room_id=get_room()
    out_volume=root_path+'/media/data/%d'%battle_id # 卷路径
    in_volume='/MyVolume'

    battle.status=2
    battle.start_time=datetime.datetime.now()
    battle.save()
    key=generate_key()
    Room.objects.create(room_id=room_id, battle_id=battle_id, key=key)

    client = docker.from_env()
    client.containers.run(image_name, command='bash /ts20/bin/run.sh %s %d'%(key,battle_id) ,tty=True, stdin_open=True, remove=True, detach=True, network_mode='host', volumes={out_volume : {'bind': in_volume}})
    return HttpResponse('Successful!')
        
def start_finals_trig(request):
    return start_finals()

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
    client.containers.run(image_name, command='bash /ts20/bin/run.sh %s %d'%(key,battle_id) ,tty=True, stdin_open=True, remove=True, detach=True, network_mode='host', volumes={out_volume : {'bind': in_volume}})
    
def add_battle(request):
    ''' 用于添加对战 '''
    # TODO:改成POST?

    if is_final:
        return HttpResponse('Final!', status=406)

    if request.method!='GET':
        return HttpResponse('Not GET!', status=406)
    # 读取request，检查合法性，并且存入Battle数据库
    team_engaged = request.GET.get('teams', None)
    robot_num = request.GET.get('AInum', None)
    initiator_id = request.GET.get('initiator_id',None)
    status = 1
    if team_engaged==None or robot_num==None or initiator_id==None :
        return HttpResponse('Lose parameters.',status=406)
    robot_num = int(robot_num)
    initiator_id = int(initiator_id)
    team_engaged = json.loads(team_engaged)

    if not 'HTTP_X_ACCESS_TOKEN' in request.META:
        return HttpResponse('401 Unauthorized', status=401)
    try:
        x_access_token = views.is_json(request.META['HTTP_X_ACCESS_TOKEN'])
        user_info = views.get_user_info(x_access_token["token"])
        team = Team.objects.get(id=initiator_id)
#if (not team.memberInTeam(user_info["id"])): # or (initiator_id not in team_engaged): NOTE
#           return HttpResponse('401 Unauthorized: You have no permission to do this.', status=401)
    except jwt.PyJWTError:
        return HttpResponse('401 Unauthorized: Token invalid or expired', status=401)
    except json.JSONDecodeError:
        return HttpResponse('422 Unprocessible Entity: JSON Decode Error.',status=422)
#except:
#return HttpResponse('401 Unauthorized lala',status=401)
    
    for team_id in team_engaged:
        team = Team.objects.filter(id=team_id)
        if team.exists()==False:
            return HttpResponse('No team:%s'%team_id,status=406)
        if team[0].valid!=15:
            return HttpResponse('Team \"%s\" is invalid!'%(team_id),status=406)
    ini_team = Team.objects.get(id=initiator_id)
    if ini_team.get_battle_time()==0:
        return HttpResponse('No remaining times.',status=405)
    ini_team.battle_time -= 1
    ini_team.save()

    battle = Battle.objects.create(team_engaged=json.dumps(team_engaged), robot_num=robot_num, status=status, initiator_id=initiator_id)
    battle_id=battle.id # 对战ID
    path = root_path+'/media/data/%d'%battle_id ;
    os.makedirs(path)
    d = {}
    cnt = 0
    for team_id in team_engaged:
        team = Team.objects.get(id=team_id)
        team_name = team.teamname
        d[cnt]=team_id
        for j in range(4):
            shutil.copyfile(so_path+'/%d_%d.so'%(team.id,j),path+'/libAI_%d_%d.so'%(cnt,j))
        cnt+=1
        if team_id!=initiator_id: # 更改对战历史
            tmp=json.loads(team.history_passive)
            tmp.append(battle_id)
            team.history_passive=json.dumps(tmp)
        else:
            tmp=json.loads(team.history_active)
            tmp.append(battle_id)
            team.history_active=json.dumps(tmp)
        team.save()
        
    for i in range(robot_num):
        #d[cnt]='__AI%d'%i # 与队伍命名不可冲突
        d[cnt]=AI_IND+cnt ;
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

def is_AI(team_id):
    return (team_id>=AI_IND)

def view_result(request):
    ''' 查询对战结果，传入battle_id，返回一个JSON '''
    if is_final:
        start_finals()
    update_rank()
    if request.method!='GET':
        return HttpResponse('Not GET!', status=406)
    battle_id = request.GET.get('battle_id', default=-1)
    query_id  = request.GET.get('team_id', default=-1)
    if battle_id==-1 or query_id==-1 :
        return HttpResponse('Lose parameter', status=406)
    query_id  = int(query_id)
    battle = Battle.objects.filter(id=battle_id)
    if battle.exists()==False :
        return HttpResponse('Not Found', status=404)
    battle     = battle[0]
    initiator  = battle.initiator_id
    status     = battle.status
    ret        = {}
    #ret['teams'] = battle.team_engaged
    ret['teams'] = []
    ret['ainum'] = battle.robot_num
    ret['state'] = status
    ret['initiator_id'] = initiator
    ret['battle_id'] = battle_id
    teams = json.loads(battle.team_engaged)
    for team_id in teams:
        ret['teams'].append(Team.objects.get(id=team_id).teamname)

    # 返回对战队伍、AI数目、冠军、发起者排名和得分、对战是否结束
    if status!=0 :
        ret['winner'] = '-1'
        ret['rank']   = -1
        ret['score']  = -1
        return JsonResponse(ret)
    result     = json.loads(battle.result)
    id_map     = json.loads(battle.id_map)
    score      = sorted(result['score'].items(), key=lambda item:item[1], reverse=True)
    champion   = id_map[score[0][0]]
    initiator_rank  = 0
    initiator_score = 0
    virtual_id = -1   # 发起者在房间中的id
    for (vid,id) in id_map.items():
        if id==query_id:
            virtual_id = vid
    if virtual_id==-1:
        return HttpResponse("No such team in this battle!", status=404)
    for i in range(0,len(score)):
        if score[i][0]==virtual_id:
            initiator_rank  = i+1
            initiator_score = score[i][1]
    if not is_AI(champion):
        ret['winner'] = Team.objects.get(id=champion).teamname
    else:
        ret['winner'] = 'AI'
    ret['rank']   = initiator_rank
    ret['score']  = initiator_score
    return JsonResponse(ret)

def update_rank():
    teams = Team.objects.all().order_by('-score')
    cnt=1
    for team in teams:
        team.rank=cnt
        team.save()
        cnt+=1

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
    json_name = None
    for name in items:
        if name.endswith('.json'):
            json_name=name
    if(json_name==None):
        battle.status=3
        team = Team.objects.get(id=battle.initiator_id)
        team.battle_time += 1
        team.save()
        battle.save()
        if is_final:
            start_finals()
        else:
            run_battle()
        return HttpResponse('Battle Error!')
    result=open(data_path+'/%d/%s'%(battle_id,json_name)).read()
    result=json.loads(result)
    result['battle_id']=battle_id
    id_map = json.loads(battle.id_map)
    game_score = list(result['score'])
    origin_score = [0 for i in range(len(game_score))]
    for i in range(len(game_score)):
        if not is_AI(int(id_map[game_score[i][0]])):
            origin_score[i] = Team.objects.get(id=int(id_map[game_score[i][0]])).score
    origin_score = update(np.array(origin_score), np.array(list(result['score'].values())))
    for i in range(len(game_score)):
        if not is_AI(int(id_map[game_score[i][0]])):
            team = Team.objects.get(id=int(id_map[game_score[i][0]]))
            team.score = origin_score[i]
            team.save()
    battle.result=json.dumps(result)
    battle.status=0

    battle.save()
    update_rank()
    if is_final:
        start_finals()
    else:
        run_battle()
    return HttpResponse('End battle successfully!')

def compile(request):
    ''' 给定队伍编号team_id以及职业编号(0~3)ind，进行代码编译并放入/media/Codes/output '''
    out_volume = root_path+'/media/temp'+generate_key(10)
    in_volume = '/MyVolume'
    team_id = int(request.GET.get('team_id',None))
    ind = request.GET.get('ind',None)
    if team_id==None or ind==None:
        return HttpResponse('Wrong parameter! Please specify the team_id and ind.')
    if Team.objects.filter(id=team_id).exists()==False:
        return HttpResponse('No such team !')
    ind = int(ind)
    origin_path = codes_path+'/%d_%d.cpp'%(team_id,ind)
    target_path = so_path+'/%d_%d.so'%(team_id,ind)
    if os.path.exists(origin_path)==False:
        return HttpResponse('No corresponding .cpp file!')
    if os.path.exists(codes_path+'/output')==False:
        os.makedirs(codes_path+'/output')
    #if os.path.isfile(target_path):
    #    os.remove(target_path)
    # 不删除原本的.so文件，避免队伍恶意拒绝参赛
    os.makedirs(out_volume)
    try:
        shutil.copyfile(origin_path, out_volume+'/code.cpp')
        
        client = docker.from_env()
        try:
            client.containers.run(image_name, command='bash /ts20/bin/compile.sh' ,tty=True, stdin_open=True, remove=True, network_mode='host', volumes={out_volume : {'bind': in_volume}})
        except docker.errors.ContainerError:
            pass
        team=Team.objects.get(id=team_id)
        status=''
        if os.path.exists(out_volume+'/out.so')==True:
            status='Compile successfully.'
            team.valid|=(1<<ind)
            shutil.copyfile(out_volume+'/out.so',target_path)
        else:
            status='Compile Error.'
        team.save()
        information=open(out_volume+'/log').read()
        ret={}
        ret['status']=status
        ret['information']=information
        shutil.rmtree(out_volume)
        return JsonResponse(ret)
    except:
        shutil.rmtree(out_volume)


def compile2(team_id, ind):
    ''' ��~Y��~Z�~X~_��~M��~V�~O�team_id以�~O~J�~A~L��~Z��~V�~O�(0~3)ind��~L��~[��~L代�| ~A��~V��~Q并�~T��~E�/media/Codes/output '''
    try:
        out_volume = root_path + '/media/temp' + generate_key(10)
        in_volume = '/MyVolume'
        team_id = int(team_id)
        ind = int(ind)
        if team_id == None or ind == None:
            return HttpResponse('Wrong parameter! Please specify the team_id and ind.')
        if Team.objects.filter(id=team_id).exists() == False:
            return HttpResponse('No such team !')
        origin_path = codes_path + '/%d_%d.cpp' % (team_id, ind)
        target_path = so_path + '/%d_%d.so' % (team_id, ind)
        if os.path.exists(origin_path) == False:
            return HttpResponse('No corresponding .cpp file!')
        if os.path.exists(codes_path + '/output') == False:
            os.makedirs(codes_path + '/output')
        # if os.path.isfile(target_path):
        #    os.remove(target_path)
        os.makedirs(out_volume)
        shutil.copyfile(origin_path, out_volume + '/code.cpp')
    
        client = docker.from_env()
        try:
            client.containers.run(image_name, command='bash /ts20/bin/compile.sh', tty=True, stdin_open=True, remove=True,
                                network_mode='host', volumes={out_volume: {'bind': in_volume}})
        except docker.errors.ContainerError:
            pass
        team = Team.objects.get(id=team_id)
        status = ''
        if os.path.exists(out_volume + '/out.so') == True:
            status = 'Compile successfully.'
            team.valid |= (1 << ind)
            shutil.copyfile(out_volume + '/out.so', target_path)
        else:
            status = 'Compile Error.'
        team.save()
        information = open(out_volume + '/log').read()
        ret = {}
        ret['status'] = status
        ret['information'] = information
        shutil.rmtree(out_volume)
        return ret
    except:
        shutil.rmtree(out_volume)
