from django.http import HttpResponse, JsonResponse, FileResponse
from django.utils.http import urlquote
from .models import *
from django.shortcuts import render,get_object_or_404
from django.views.decorators.csrf import csrf_exempt
from django.template import *
import requests
import hashlib, uuid
from django.db.models import Q
import django.utils.timezone as tzd
import datetime, time
import jwt
from django.core.files.storage import FileSystemStorage
from django.conf import settings

EXPIRE_TIME = 14400
def make_x_access_token(eesast_token, validDuration=EXPIRE_TIME):
    user_info = jwt.decode(eesast_token, verify=False)
    token_content = dict()
    token_content["eesast"] = eesast_token
    token_content["iat"] = time.time()
    token_content["exp"] = time.time() + validDuration
    if user_info["exp"] < token_content["exp"]:
        raise jwt.ExpiredSignatureError
    x_access_token =  jwt.encode(token_content, settings.SECRET_KEY, algorithm='HS256')
    user_info["auth"] = True
    user_info["token"] = x_access_token.decode('utf-8')
    del user_info["name"], user_info["email"], user_info["group"], user_info["role"], user_info["iat"], user_info["exp"]
    return user_info

def get_user_info(x_access_token):
    token_content = jwt.decode(x_access_token, settings.SECRET_KEY, algorithms='HS256')
    user_info = jwt.decode(token_content["eesast"], verify=False)
    user_info["id"] = int(user_info["id"])
    user_info["eesast"] = token_content["eesast"]
    return user_info

def is_json(myjson):
    if(isinstance(myjson, str)):
        json_object = json.loads(myjson)
    else:
        json_object = json.loads(myjson.decode('utf-8'))
    return json_object

def get_error_msg(err_status):
    msg_list = dict()
    msg_list[405] = "405 Method Not Allowed."
    msg_list[404] = "404 Not Found: User does not exist."
    msg_list[422] = "422 Unprocessible Entity: Missing essential POST data."
    msg_list[4221] = "422 Unprocessible Entity: JSON Decode Error."
    msg_list[4012] = "{\"auth\":false, \"token\":\"\"}"
    msg_list[4011] = "401 Unauthorized: Token invalid or expired."
    msg_list[401] = "401 Unauthorized."
    msg_list[409] = "409 Conflict: Team name already exists."
    return msg_list.get(err_status, str(err_status) + " Unknown Error.")

def index(request):
    global_settings = GlobalSetting.objects.all()
    announcements = Announcement.objects.all()
    context = {
        'global_settings': global_settings,
        'announcements': announcements,
    }
    return render(request, 'index.html', context)
def detail(request, post_id):
    announcement = get_object_or_404(Announcement, pk=post_id)
    return render(request, 'detail.html', {'post': announcement})

@csrf_exempt
def auth(request):
    try:
        assert request.method == 'POST', 405
        auth_URL = 'https://api.eesast.com/v1/users/login'
        login_credentials = is_json(request.body)
        login_response = requests.post(auth_URL, json=login_credentials)
        assert login_response.status_code == 200, login_response.status_code
        eesast_res = login_response.json()
        output = make_x_access_token(eesast_res["token"])
        response = JsonResponse(output, status=200)
    except AssertionError as error:
        err_status = int(error.__str__())
        err_get_msg_id = 4012 if err_status==401 else err_status
        msg = get_error_msg(err_get_msg_id)
        response = HttpResponse(msg, status=err_status)
    except json.JSONDecodeError:
        msg = get_error_msg(4221)
        response = HttpResponse(msg, status=422)
    #else:
       # response = HttpResponse("520 Unknown Error", status=520)
    return response


@csrf_exempt
def users(request):
    try:
        if request.method == 'POST':
            add_URL = 'https://api.eesast.com/v1/users'
            user_profile = is_json(request.body)
            add_response = requests.post(add_URL, json=user_profile)
            assert add_response.status_code == 201, add_response.status_code
            auth_URL = 'https://api.eesast.com/v1/users/login'
            login_credentials = {'username': user_profile["username"], 'password': user_profile["password"]}
            login_response = requests.post(auth_URL, json=login_credentials)
            assert login_response.status_code == 200, login_response.status_code
            eesast_res = login_response.json()
            output = make_x_access_token(eesast_res["token"])
            response = JsonResponse(output, status=200)
        elif request.method == 'GET':
            assert 'HTTP_X_ACCESS_TOKEN' in request.META, 4011
            x_access_token = is_json(request.META['HTTP_X_ACCESS_TOKEN'])
            assert type(x_access_token) is dict, 401
            assert x_access_token["token"],401
            user_info = get_user_info(x_access_token["token"])
            get_user_URL = 'https://api.eesast.com/v1/users?detailInfo=True'
            get_user_hed = {'Authorization': 'Bearer ' + user_info["eesast"]}
            get_user_response = requests.get(get_user_URL, headers=get_user_hed)
            response = HttpResponse(get_user_response.text, status=get_user_response.status_code)
        else:
            assert False, 405
    except AssertionError as error:
        err_status = int(error.__str__())
        msg = get_error_msg(err_status)
        err_status = 401 if err_status==4011 else err_status
        response = HttpResponse(msg, status=err_status)
    except jwt.PyJWTError:
        msg = get_error_msg(4011)
        response = HttpResponse(msg, status=401)
    except json.JSONDecodeError:
        msg = get_error_msg(4221)
        response = HttpResponse(msg, status=422)
    #else:
      #  response = HttpResponse("520 Unknown Error", status=520)
    return response


@csrf_exempt
def modifyUser(request, user_id):
    target_user_id = int(user_id)
    target_user_URL = 'https://api.eesast.com/v1/users/' + str(user_id) + '?detailInfo=true'
    try:
        assert 'HTTP_X_ACCESS_TOKEN' in request.META, 4011
        x_access_token = is_json(request.META['HTTP_X_ACCESS_TOKEN'])
        assert x_access_token["token"], 401
        user_info = get_user_info(x_access_token["token"])
        head = {'Authorization': 'Bearer ' + user_info["eesast"]}
        if request.method == 'GET':
            query_response = requests.get(target_user_URL, headers=head)
            response = HttpResponse(query_response.text, status=query_response.status_code)
        elif request.method == 'PUT':
            modifyInfo = is_json(request.body)
            query_response = requests.put(target_user_URL, json=modifyInfo, headers=head)
            msg = "204 OK." if (query_response.status_code==204) else (query_response.text)
            response = HttpResponse(msg, status=query_response.status_code)
        elif request.method == 'DELETE':
            assert user_info["role"] == "root", 401
            query_response = requests.delete(target_user_URL, headers=head)
            response = HttpResponse(query_response.text, status=query_response.status_code)
        else:
            assert False, 405
    except AssertionError as error:
        err_status = int(error.__str__())
        msg = get_error_msg(err_status)
        err_status = 401 if err_status == 4011 else err_status
        response = HttpResponse(msg, status=err_status)
    except jwt.PyJWTError:
        msg = get_error_msg(4011)
        response = HttpResponse(msg, status=401)
    except json.JSONDecodeError:
        msg = get_error_msg(4221)
        response = HttpResponse(msg, status=422)
    #else:
      #  response = HttpResponse("520 Unknown Error", status=520)
    return response


def append_team_member_name(user_info, query, showtype=0):
    output = list()
    get_name_head = {'Authorization': 'Bearer ' + user_info["eesast"]}
    for team in query:
        if (team.memberInTeam(user_info["id"])):
            showtype = 2
        thisTeamInfo = team.get_teamInfo(showtype)
        thisTeamInfo["members"] = list()
        try:
            for member in thisTeamInfo["membersID"]:
                get_name_URL = 'https://api.eesast.com/v1/users/' + str(member)
                query_response = requests.get(get_name_URL, headers=get_name_head)
                assert query_response.status_code == 200, query_response.status_code
                query_result = is_json(query_response.content)
                assert type(query_result) is dict, 500
                assert 'name' in query_result, 500
                thisTeamInfo["members"].append(query_result["name"])
                if str(member) == str(thisTeamInfo["captainID"]):
                    thisTeamInfo["captain"] = query_result["name"]
        except AssertionError:
            thisTeamInfo["captain"] = thisTeamInfo["captainID"]
            thisTeamInfo["members"] = thisTeamInfo["membersID"]
        output.append(thisTeamInfo)
    return output


def get_teamid_by_userid(user_id):
    user_id = int(user_id)
    query = Team.objects.filter(Q(members__contains=user_id))
    if (query.count() > 0):
        for eachteam in query:
            if user_id in eachteam.get_member():
                return eachteam.pk
        return -1
    else:
        return -1
    pass

@csrf_exempt
def teams(request):
    try:
        assert 'HTTP_X_ACCESS_TOKEN' in request.META, 4011
        x_access_token = is_json(request.META['HTTP_X_ACCESS_TOKEN'])
        assert x_access_token["token"], 401
        user_info = get_user_info(x_access_token["token"])
        #head = {'Authorization': 'Bearer ' + user_info["eesast"]}
        if request.method == 'GET':
            if 'detailInfo' in request.GET:
                showtype = int(request.GET['detailInfo'] == 'True' or request.GET['detailInfo'] == 'true')
            else:
                showtype = 0
            query = Team.objects.all()
            output = append_team_member_name(user_info, query, showtype)
            response = JsonResponse(output, status=200, safe=False)
        elif request.method == 'POST':
            new_team_info = is_json(request.body)
            assert 'teamname' in new_team_info and 'description' in new_team_info , 422
            new_team_info['captain'] = int(user_info['id'])
            new_team_info['members'] = json.dumps([int(new_team_info['captain'])])
            new_team_info['invitecode'] = hashlib.sha512(((hashlib.sha512(new_team_info['teamname'].encode('utf-8')).hexdigest()) + uuid.uuid4().hex).encode('utf-8')).hexdigest()[0:9]
            assert Team.objects.filter(teamname = new_team_info['teamname']).count()==0, 409
            if (get_teamid_by_userid(int(user_info["id"]))>=0):
                response = HttpResponse("409 Conflict: User is already in a team.", status=409)
            else:
                result = Team.objects.create(**new_team_info)
                returnInfo = {'invitecode': result.invitecode}
                response = JsonResponse(returnInfo, status=200)
        else:
            assert False, 405
    except AssertionError as error:
        err_status = int(error.__str__())
        msg = get_error_msg(err_status)
        err_status = 401 if err_status == 4011 else err_status
        response = HttpResponse(msg, status=err_status)
    except jwt.PyJWTError:
        msg = get_error_msg(4011)
        response = HttpResponse(msg, status=401)
    except json.JSONDecodeError:
        msg = get_error_msg(4221)
        response = HttpResponse(msg, status=422)
    #else:
      #  response = HttpResponse("520 Unknown Error", status=520)
    return response


@csrf_exempt
def modifyTeamByID(request, teamid):
    try:
        assert 'HTTP_X_ACCESS_TOKEN' in request.META, 4011
        x_access_token = is_json(request.META['HTTP_X_ACCESS_TOKEN'])
        assert x_access_token["token"], 401
        user_info = get_user_info(x_access_token["token"])
        if request.method == 'GET':
            query = Team.objects.filter(pk=teamid)
            if 'detailInfo' in request.GET:
                showtype = int(request.GET['detailInfo'] == 'True' or request.GET['detailInfo'] == 'true')
            else:
                showtype = 0
            output = append_team_member_name(user_info, query, showtype)
            if (query.count() == 0):
                response = HttpResponse("404 Not Found: No record for requested team number.", status=404)
            else:
                response = JsonResponse(output[0], status=200)
        elif request.method == 'PUT':
            target_team = Team.objects.get(pk=teamid)
            has_permission = 0;
            if int(target_team.captain) == int(user_info["id"]) or user_info["role"] == "root":
                has_permission = 1
            assert has_permission, 401
            content_to_update = is_json(request.body)
            add_err = 0
            if "teamname" in content_to_update:
                assert Team.objects.filter(teamname=content_to_update["teamname"]).count() == 0, 409
                target_team.teamname = content_to_update["teamname"]
            if "description" in content_to_update:
                target_team.description = content_to_update["description"]
            if "captain" in content_to_update:
                content_to_update["captain"] = int(content_to_update["captain"])
                if target_team.memberInTeam(content_to_update["captain"]):
                    target_team.captain = content_to_update["captain"]
                else:
                    add_err = 1
            target_team.save()
            response = HttpResponse("204 Successful Operation.", status=204)
            if add_err:
                response = HttpResponse("400 Bad Request: New captain is NOT a member in this team.", status=400)
        elif request.method == 'DELETE':
            target_team = Team.objects.get(pk=teamid)
            has_permission = 0;
            if int(target_team.captain) == int(user_info["id"]) or user_info["role"] == "root" :
                has_permission = 1
            assert has_permission, 401
            target_team.delete()
            response = HttpResponse("204 Deleted.", status=204)
        else:
            assert False, 405
    except AssertionError as error:
        err_status = int(error.__str__())
        msg = get_error_msg(err_status)
        err_status = 401 if err_status == 4011 else err_status
        response = HttpResponse(msg, status=err_status)
    except jwt.PyJWTError:
        msg = get_error_msg(4011)
        response = HttpResponse(msg, status=401)
    except json.JSONDecodeError:
        msg = get_error_msg(4221)
        response = HttpResponse(msg, status=422)
    except Team.DoesNotExist:
        response = HttpResponse("404 Not Found: No record for requested team number.", status=404)
    #else:
      #  response = HttpResponse("520 Unknown Error", status=520)
    return response


@csrf_exempt
def modifyTeamMembersByID(request, teamid):
    try:
        assert 'HTTP_X_ACCESS_TOKEN' in request.META, 4011
        x_access_token = is_json(request.META['HTTP_X_ACCESS_TOKEN'])
        assert x_access_token["token"], 401
        user_info = get_user_info(x_access_token["token"])
        if request.method == 'GET':
            query = Team.objects.filter(pk=teamid)
            output = append_team_member_name(user_info, query, 0)
            if (query.count() == 0):
                response = HttpResponse("404 Not Found: No record for requested team number.", status=404)
            else:
                response = JsonResponse(output[0]["members"], status=200, safe=False)
        elif request.method == 'POST':
            targetTeam = Team.objects.get(pk=teamid)
            addInfo = is_json(request.body)
            assert type(addInfo) is dict, 422
            assert 'invitecode' in addInfo, 422
            error = 0
            if (targetTeam.member_num >= targetTeam.get_member_limit()):
                error = 1
                response = HttpResponse("409 Conflict: The number of members exceeds.", status=409)
            elif (str(targetTeam.invitecode) != str(addInfo["invitecode"])):
                error = 1
                response = HttpResponse("403 Forbidden: Incorrect invite code.", status=403)
            elif (get_teamid_by_userid(int(user_info["id"]))>=0):
                error = 1
                response = HttpResponse("409 Conflict: User is already in a team.", status=409)
            if not error:
                targetTeam.add_member(int(user_info["id"]))
                response = HttpResponse("201 Operation Successful", status=201)
        elif request.method == 'DELETE':
            a = get_teamid_by_userid(user_info["id"])
            assert a == teamid, 401
            targetTeam = Team.objects.get(pk=a)
            if (str(targetTeam.captain) == str(user_info["id"])):
                error = 1
                response = HttpResponse("400 Bad Request: Captain cannot be deleted.", status=400)
            else:
                targetTeam.delete_member(user_info["id"])
                response = HttpResponse("204 Operation Successful.", status=204)
        else:
            assert False, 405
    except AssertionError as error:
        err_status = int(error.__str__())
        msg = get_error_msg(err_status)
        err_status = 401 if err_status == 4011 else err_status
        response = HttpResponse(msg, status=err_status)
    except jwt.PyJWTError:
        msg = get_error_msg(4011)
        response = HttpResponse(msg, status=401)
    except json.JSONDecodeError:
        msg = get_error_msg(4221)
        response = HttpResponse(msg, status=422)
    except Team.DoesNotExist:
        response = HttpResponse("404 Not Found: No record for requested team number.", status=404)
    # else:
    #  response = HttpResponse("520 Unknown Error", status=520)
    return response



@csrf_exempt
def personalTeamActions(request, teamid, userid):
    try:
        assert 'HTTP_X_ACCESS_TOKEN' in request.META, 4011
        x_access_token = is_json(request.META['HTTP_X_ACCESS_TOKEN'])
        assert x_access_token["token"], 401
        user_info = get_user_info(x_access_token["token"])
        if request.method == 'GET':
            a = get_teamid_by_userid(userid)
            if(a!=-1):
                target_team = Team.objects.filter(pk=a)
                output = append_team_member_name(user_info, target_team, 1)
                response = JsonResponse(output[0], status=200)
            else:
                response = HttpResponse("404 Not Found: The user is not in any of the team.", status=404)
        elif request.method == 'DELETE':
            target_team = Team.objects.get(pk=teamid)
            has_permission = 0;
            if int(target_team.captain) == int(user_info["id"]) or user_info["role"] == "root" :
                has_permission = 1
            assert has_permission, 401
            if (str(target_team.captain) == str(userid)):
                response = HttpResponse("400 Bad Request: Captain cannot be deleted.", status=400)
            elif(not target_team.memberInTeam(int(userid))):
                response = HttpResponse("400 Bad Request: The ID is not in the team.", status=400)
            else:
                target_team.delete_member(userid)
                response = HttpResponse("204 Operation Successful.", status=204)
        else:
            assert False, 405
    except AssertionError as error:
        err_status = int(error.__str__())
        msg = get_error_msg(err_status)
        err_status = 401 if err_status == 4011 else err_status
        response = HttpResponse(msg, status=err_status)
    except jwt.PyJWTError:
        msg = get_error_msg(4011)
        response = HttpResponse(msg, status=401)
    except json.JSONDecodeError:
        msg = get_error_msg(4221)
        response = HttpResponse(msg, status=422)
    except Team.DoesNotExist:
        response = HttpResponse("404 Not Found: No record for requested team number.", status=404)
    # else:
    #  response = HttpResponse("520 Unknown Error", status=520)
    return response



get_globalSettings = GlobalSetting.objects.all()
submission = dict()
if get_globalSettings.count()==1:
    submission["start"] = tzd.localtime(get_globalSettings[0].submission_start)
    submission["end"] = tzd.localtime(get_globalSettings[0].submission_end)
else:
    submission = False
def systemOpen():
    now = datetime.datetime.now().replace(tzinfo=tzd.get_current_timezone())
    if submission == False:
        return False
    if now < submission["start"] or now > submission["end"]:
        return False
    else:
        return True

@csrf_exempt
def modifyTeamCodes(request, teamid):
    try:
        assert 'HTTP_X_ACCESS_TOKEN' in request.META, 4011
        x_access_token = is_json(request.META['HTTP_X_ACCESS_TOKEN'])
        assert x_access_token["token"], 401
        user_info = get_user_info(x_access_token["token"])
        target_team = Team.objects.get(pk=teamid)
        assert user_info["role"] == "root" or target_team.memberInTeam(int(user_info["id"])), 401
        if request.method == 'GET':
            pass
        elif request.method == 'POST':
            if systemOpen():
                upload_file = dict()
                if 'code0' in request.FILES:
                    upload_file[0] = request.FILES['code0']
                fs = FileSystemStorage(location=settings.MEDIA_ROOT+'/Codes')
                for code_type, code_file in upload_file.items():
                    filename = str(teamid) + '_' +str(code_type) +'.cpp'
                    f = fs.save(filename, code_file)
                    response = HttpResponse(f, status=200)
            else:
                response = HttpResponse("Forbidden", status=403)
        else:
            assert False, 405
    except AssertionError as error:
        err_status = int(error.__str__())
        msg = get_error_msg(err_status)
        err_status = 401 if err_status == 4011 else err_status
        response = HttpResponse(msg, status=err_status)
    except jwt.PyJWTError:
        msg = get_error_msg(4011)
        response = HttpResponse(msg, status=401)
    except json.JSONDecodeError:
        msg = get_error_msg(4221)
        response = HttpResponse(msg, status=422)
    except Team.DoesNotExist:
        response = HttpResponse("404 Not Found: No record for requested team number.", status=404)
    # else:
    #  response = HttpResponse("520 Unknown Error", status=520)
    return response



def listAnnouncementAPI(request):
    response = HttpResponse("405 Method not allowed: You\'ve used an unallowed method.", status=405)
    if request.method == 'GET':
        query = Announcement.objects.all()
        output = list()
        for annoucement in query:
            output.append(annoucement.get_AnnouncementInfo(0))
        response = JsonResponse(output, status=200, safe=False)
    return response


def viewAnnouncementAPI(request, post_id):
    response = HttpResponse("405 Method not allowed: You\'ve used an unallowed method.", status=405)
    if request.method == 'GET':
        try:
            query = Announcement.objects.get(pk=post_id)
            output = list()
            output.append(query.get_AnnouncementInfo(1))
            response = JsonResponse(output, status=200, safe=False)
        except Announcement.DoesNotExist:
            response = HttpResponse("404 Not found.", status = 404)
    return response


def listFileAPI(request):
    response = HttpResponse("405 Method not allowed: You\'ve used an unallowed method.", status=405)
    if request.method == 'GET':
        query = File.objects.all()
        output = list()
        for file in query:
            output.append(file.get_FileInfo())
        response = JsonResponse(output, status=200, safe=False)
    return response


def downloadFileAPI(request, file_id):
    response = HttpResponse("405 Method not allowed: You\'ve used an unallowed method.", status=405)
    if request.method == 'GET':
        try:
            query = File.objects.get(pk=file_id)
            response = FileResponse(open(query.content.path, 'rb'), status=200)
            response['Content-Type'] = 'application/octet-stream'
            response['Content-Disposition'] = 'attachment;filename = ' + urlquote(query.filename())
        except File.DoesNotExist:
            response = HttpResponse("404 Not found.", status = 404)
    return response

def getGlobalSettings(request):
    response = HttpResponse("405 Method not allowed: You\'ve used an unallowed method.", status=405)
    if request.method == 'GET':
        query = GlobalSetting.objects.all()
        if query.count()==1:
            response = JsonResponse(query[0].get_globalSetting(), status=200)
        else:
            response = HttpResponse("404 Not found: Global Settings of this event are not yet configured.", status=404)
    return response