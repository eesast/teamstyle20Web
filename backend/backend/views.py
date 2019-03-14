from django.http import HttpResponse, JsonResponse, FileResponse
from django.utils.http import urlquote
from .models import *
from django.shortcuts import render,get_object_or_404
from django.views.decorators.csrf import csrf_exempt
from django.template import *
import requests
import hashlib, uuid
from django.db.models import Q
from django.conf import settings
# Create your views here.


def is_json(myjson):
    try:
        if(isinstance(myjson, str)):
            json_object = json.loads(myjson)
        else:
            json_object = json.loads(myjson.decode('utf-8'))
    except ValueError:
        return False
    return json_object



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
    if request.method == 'POST':
        authURL = 'https://api.eesast.com/v1/users/login'
        login_credentials = is_json(request.body)
        response = HttpResponse("422 Unprocessable Entity: Missing essential post data.", status=422)
        if type(login_credentials) is dict:
            if 'username' in login_credentials and 'password' in login_credentials:
                if login_credentials["username"] and login_credentials["password"]:
                    token_response = requests.post(authURL, json=login_credentials)
                    if token_response.status_code == 200:
                        x_access_token = token_response.json()
                        x_access_token["auth"] = True
                        x_access_token["username"] = login_credentials["username"]
                        getID = 'https://api.eesast.com/v1/users?username=' + login_credentials["username"] + '&detailInfo=True'
                        getID_hed = {'Authorization': 'Bearer ' + x_access_token["token"]}
                        user_response = requests.get(getID, headers=getID_hed)
                        x_access_token["id"] = -1
                        if user_response.status_code == 200:
                            for user in user_response.json():
                                if user["username"] == login_credentials["username"]:
                                    x_access_token["id"] = user["id"]
                                    break
                        elif user_response.status_code == 401:
                            x_access_token["id"] = "401:Invalid or expired token or permission denied."
                        response = JsonResponse(x_access_token, status=200)
                    elif token_response.status_code == 404:
                        response = HttpResponse("404 Not Found: User does not exist.", status=404)
                    elif token_response.status_code == 401:
                        response = JsonResponse({'auth': False, 'token': ''}, status=401)
                    else:
                        response = HttpResponse("520 Unknown Error", status=520)
    else:
        response = HttpResponse("405 Method Not Allowed", status=405)
    return response


@csrf_exempt
def users(request):
    if request.method == 'POST':
        user_profile = is_json(request.body)
        addurl = 'https://api.eesast.com/v1/users'
        if type(user_profile) is dict:
            add_response = requests.post(addurl, json = user_profile)
            if add_response.status_code == 201:
                login_credentials = {'username' : user_profile["username"], 'password': user_profile["password"]}
                if login_credentials["username"] and login_credentials["password"]:
                    authURL = 'https://api.eesast.com/v1/users/login'
                    token_response = requests.post(authURL, json=login_credentials)
                    if token_response.status_code == 200:
                        x_access_token = token_response.json()
                        x_access_token["auth"] = True
                        x_access_token["username"] = login_credentials["username"]
                        x_access_token["id"] = user_profile["id"]
                        response = JsonResponse(x_access_token, status=201)
                    else:
                        response = HttpResponse("Unexpected error occured when acquiring token: " + token_response.text, status = token_response.status_code)
            elif add_response.status_code == 422:
                response = HttpResponse("422 Unprocessable Entity: Missing form data.", status=422)
            else:
                response = HttpResponse("Unexpected: " + add_response.text, status=add_response.status_code)
        else:
            response = HttpResponse("422 Unprocessable Entity: Error in JSON format.", status=422)
    elif request.method == 'GET':
        response = HttpResponse("401 Unauthorized: Invalid or expired token.", status=401)
        if 'HTTP_X_ACCESS_TOKEN' in request.META :
            x_access_token_json = request.META['HTTP_X_ACCESS_TOKEN']
            x_access_token = is_json(x_access_token_json)
            if type(x_access_token) is dict:
                if 'auth' in x_access_token and 'token' in x_access_token:
                    if x_access_token["auth"] == True:
                        getID = 'https://api.eesast.com/v1/users?detailInfo=True'
                        getID_hed = {'Authorization': 'Bearer ' + x_access_token["token"]}
                        user_response = requests.get(getID, headers=getID_hed)
                        response = HttpResponse(user_response.text, status=user_response.status_code)
    else:
        response = HttpResponse("405 Method Not Allowed", status=405)
    return response



@csrf_exempt
def modifyUser(request, user_id):
    targetURL = 'https://api.eesast.com/v1/users/' + str(user_id) + '?detailInfo=true'
    if request.method == 'GET':
        response = HttpResponse("401 Unauthorized: Token required.", status=401)
        if 'HTTP_X_ACCESS_TOKEN' in request.META :
            x_access_token_json = request.META['HTTP_X_ACCESS_TOKEN']
            x_access_token = is_json(x_access_token_json)
            if type(x_access_token) is dict:
                if 'auth' in x_access_token:
                    if x_access_token["auth"] == True:
                        head = {'Authorization': 'Bearer ' + x_access_token["token"]}
                        query_response = requests.get(targetURL, headers = head)
                        response = HttpResponse(query_response.text, status=query_response.status_code)
                    elif x_access_token["auth"] == False:
                        response = HttpResponse("401 Unauthorized: Invalid or expired token.", status=401)
    elif request.method == 'PUT':
        if 'HTTP_X_ACCESS_TOKEN' in request.META:
            x_access_token_json = request.META['HTTP_X_ACCESS_TOKEN']
            x_access_token = is_json(x_access_token_json)
            response = HttpResponse("401 Unauthorized: Token required.", status=401)
            if type(x_access_token) is dict:
                if 'auth' in x_access_token and 'token' in x_access_token:
                    if x_access_token["auth"] == True:
                        head = {'Authorization': 'Bearer ' + x_access_token["token"]}
                        modifyInfo = is_json(request.body)
                        query_response = requests.put(targetURL,json=modifyInfo, headers=head)
                        if query_response.status_code == 204:
                            query_response = requests.get(targetURL, headers = head)
                            response = HttpResponse(query_response.text, status=204)
                        else:
                            response = HttpResponse(query_response.text, status=query_response.status_code)
                    elif x_access_token["auth"] == False:
                        response = HttpResponse("401 Unauthorized: Invalid or expired token.", status=401)
                else:
                    response = HttpResponse("401 Unauthorized: Invalid token.", status=401)
    elif request.method == 'DELETE':
        if 'HTTP_X_ACCESS_TOKEN' in request.META :
            x_access_token_json = request.META['HTTP_X_ACCESS_TOKEN']
            x_access_token = is_json(x_access_token_json)
            response = HttpResponse("401 Unauthorized: Token required.", status=401)
            if type(x_access_token) is dict:
                if 'auth' in x_access_token and 'token' in x_access_token:
                    if x_access_token["auth"] == True:
                        head = {'Authorization': 'Bearer ' + x_access_token["token"]}
                        query_response = requests.delete(targetURL, headers = head)
                        response = HttpResponse(query_response.text, status=query_response.status_code)
                    elif x_access_token["auth"] == False:
                        response = HttpResponse("401 Unauthorized: Invalid or expired token.", status=401)
        else:
            response = HttpResponse("401 Unauthorized: Token required.", status=401)
    else:
        response = HttpResponse("405 Method Not Allowed", status=405)
    return response


from backend.models import Team

@csrf_exempt
def teams(request):
    response = HttpResponse("405 Method not allowed: You\'ve used an unallowed method.", status=405)
    if request.method == 'POST':
        response = HttpResponse("401 Unauthorized: Token required.", status=401)
        if 'HTTP_X_ACCESS_TOKEN' in request.META:
            response = HttpResponse("401 Unauthorized: Invalid or expired token.", status=401)
            x_access_token = is_json(request.META['HTTP_X_ACCESS_TOKEN'])
            if type(x_access_token) is dict:
                if 'auth' in x_access_token and 'token' in x_access_token:
                    if x_access_token["auth"] == True:
                        newTeamInfo = is_json(request.body)
                        response = HttpResponse("422 Unprocessable Entity: Missing essential post data.", status=422)
                        if 'teamname' in newTeamInfo and 'description' in newTeamInfo:
                            if newTeamInfo['teamname'] and newTeamInfo['description']:
                                newTeamInfo['captain'] = x_access_token['id']
                                newTeamInfo['members'] = json.dumps([newTeamInfo['captain']])
                                newTeamInfo['invitecode'] = hashlib.sha512(((hashlib.sha512(newTeamInfo['teamname'].encode('utf-8')).hexdigest())+uuid.uuid4().hex).encode('utf-8')).hexdigest()[0:9]
                                if(Team.objects.filter(teamname = newTeamInfo['teamname']).count()>0):
                                    response = HttpResponse("409 Conflict: Team name already exists.", status=409)
                                else:
                                    query = Team.objects.filter(Q(members__contains=x_access_token['id']))
                                    createIndicator = True
                                    if(query.count()>0):
                                        for eachteam in query:
                                            if x_access_token['id'] in eachteam.get_member():
                                                createIndicator = False
                                                response = HttpResponse("409 Conflict: User is already in a team.", status=409)
                                                break
                                    if(createIndicator):
                                        result = Team.objects.create(**newTeamInfo)
                                        returnInfo={'invitecode':result.invitecode}
                                        response = JsonResponse(returnInfo, status=200)
    elif request.method == 'GET':
        if 'detailInfo' in request.GET:
            showtype = int(request.GET['detailInfo'] == 'True' or request.GET['detailInfo'] == 'true')
        else:
            showtype = 0
        response = HttpResponse("401 Unauthorized: Token required.", status=401)
        if 'HTTP_X_ACCESS_TOKEN' in request.META:
            response = HttpResponse("401 Unauthorized: Invalid or expired token.", status=401)
            x_access_token = is_json(request.META['HTTP_X_ACCESS_TOKEN'])
            if type(x_access_token) is dict:
                if 'auth' in x_access_token and 'token' in x_access_token:
                    if x_access_token["auth"] == True:
                        query = Team.objects.all()
                        output = list()
                        for team in query:
                            if(team.memberInTeam(x_access_token["id"])):
                                output.append(team.get_teamInfo(2))
                            else:
                                output.append(team.get_teamInfo(showtype))
                        response = JsonResponse(output, status=200, safe= False)
    return response

@csrf_exempt
def modifyTeamByID(request, teamid):
    response = HttpResponse("405 Method not allowed: You\'ve used an unallowed method.", status=405)
    if request.method == 'GET':
        if 'detailInfo' in request.GET:
            showtype = int(request.GET['detailInfo'] == 'True' or request.GET['detailInfo'] == 'true')
        else:
            showtype = 0
        response = HttpResponse("401 Unauthorized: Token required.", status=401)
        if 'HTTP_X_ACCESS_TOKEN' in request.META:
            response = HttpResponse("401 Unauthorized: Invalid or expired token.", status=401)
            x_access_token = is_json(request.META['HTTP_X_ACCESS_TOKEN'])
            if type(x_access_token) is dict:
                if 'auth' in x_access_token and 'token' in x_access_token:
                    if x_access_token["auth"] == True:
                        query = Team.objects.filter(id=teamid)
                        output = list()
                        for team in query:
                            if(team.memberInTeam(x_access_token["id"])):
                                output.append(team.get_teamInfo(2))
                            else:
                                output.append(team.get_teamInfo(showtype))
                        response = JsonResponse(output, status=200, safe= False)
                        if(query.count() == 0):
                            response = HttpResponse("404 Not Found: No record for requested team number.", status=404)
    elif request.method == 'DELETE':
        try:
            targetTeam = Team.objects.get(pk=teamid)
            response = HttpResponse("401 Unauthorized: Invalid or expired token.", status=401)
            if 'HTTP_X_ACCESS_TOKEN' in request.META:
                x_access_token = is_json(request.META['HTTP_X_ACCESS_TOKEN'])
                if type(x_access_token) is dict:
                    if 'auth' in x_access_token and 'token' in x_access_token:
                        if x_access_token["auth"] == True:
                            targetURL = 'https://api.eesast.com/v1/users/' + str(x_access_token["id"]) + "?detailInfo=True"
                            head = {'Authorization': 'Bearer ' + x_access_token["token"]}
                            query_response = requests.get(targetURL, headers=head)
                            userInfo = json.loads(query_response.content)
                            has_permission = 0;
                            response = HttpResponse("401 Unauthorized: Permission Denied.", status=401)
                            if type(userInfo) is dict:
                                if userInfo["group"] == 'admin':
                                    has_permission = 1
                                if str(targetTeam.captain) == str(userInfo["id"]):
                                    has_permission = 1
                            if has_permission:
                                targetTeam.delete()
                                response = HttpResponse("204 Deleted.", status=204)
                            else:
                                response = HttpResponse("400 Bad Request: The user is neither the captain of the team nor the admin.", status=400)
        except Team.DoesNotExist:
            response = HttpResponse("404 Not found: No such team.", status=404)
        return response
    elif request.method == 'PUT':
        try:
            targetTeam = Team.objects.get(pk=teamid)
            response = HttpResponse("401 Unauthorized: Invalid or expired token.", status=401)
            if 'HTTP_X_ACCESS_TOKEN' in request.META:
                x_access_token = is_json(request.META['HTTP_X_ACCESS_TOKEN'])
                if type(x_access_token) is dict:
                    if 'auth' in x_access_token and 'token' in x_access_token:
                        if x_access_token["auth"] == True:
                            targetURL = 'https://api.eesast.com/v1/users/' + str(x_access_token["id"]) + "?detailInfo=True"
                            head = {'Authorization': 'Bearer ' + x_access_token["token"]}
                            query_response = requests.get(targetURL, headers=head)
                            userInfo = json.loads(query_response.content)
                            has_permission = 0;
                            response = HttpResponse("401 Unauthorized: Permission Denied.", status=401)
                            if type(userInfo) is dict:
                                if userInfo["group"] == 'admin':
                                    has_permission = 1
                                if str(targetTeam.captain) == str(userInfo["id"]):
                                    has_permission = 1
                            add_captain_err = 0
                            if has_permission:
                                contentToUpdate = is_json(request.body)
                                if type(contentToUpdate) is dict:
                                    if "teamname" in contentToUpdate:
                                        targetTeam.teamname = contentToUpdate["teamname"]
                                    if "description" in contentToUpdate:
                                        targetTeam.description = contentToUpdate["description"]
                                    if "captain" in contentToUpdate:
                                        if targetTeam.memberInTeam(contentToUpdate["captain"]):
                                            targetTeam.captain = contentToUpdate["captain"]
                                        else:
                                            add_captain_err = 1
                                    targetTeam.save()
                                response = HttpResponse("204 Successful Operation.", status=204)
                                if add_captain_err:
                                    response = HttpResponse("400 Bad Request: New captain is NOT a member in this team.", status=400)
                            else:
                                response = HttpResponse("400 Bad Request: The user is neither the captain of the team nor the admin.", status=400)
        except Team.DoesNotExist:
            response = HttpResponse("404 Not found: No such team.", status=404)
        return response
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