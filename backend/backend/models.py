# yyr: 在Team一个IntegerField（valid）和id，还有Battle的status以及新增id_map和initiator_name
from django.db import models
import json
from django.core.exceptions import ValidationError
import django.utils.timezone as tzd
import os
import requests

HISTORY_LENGTH = 20
MEMBER_ALLOWED = 3

class Team(models.Model):
    id = models.AutoField(primary_key= True, verbose_name="Team ID")
    teamname = models.CharField(max_length=30, null=True, verbose_name='Team name')
    description = models.TextField( null=True, verbose_name='Description')
    captain = models.CharField(max_length=50, null=True, verbose_name='Captain')
    members = models.TextField(default='[]', verbose_name='Team members',help_text="Student IDs of team members in JSON array format.  If this is to be modified on django admin site, please make sure it retains valid in JSON format.")
    member_num = models.IntegerField(default = 1, null=True, verbose_name='Members count')
    invitecode = models.CharField(max_length=20, null=True, verbose_name='Invitation code')
    createAt = models.DateTimeField(auto_now_add=True, verbose_name='Create Time')
    score = models.IntegerField(default = -1)
    rank = models.IntegerField(default = 999999)
    battle_time = models.IntegerField(default = 1)
    codes = models.TextField(null=True, blank = True)
    history_active = models.TextField(default='[]', verbose_name='Active fighting history', help_text="Fighting history records of the team in JSON format.  If this is to be modified on django admin site, please make sure it retains valid in JSON format.")
    history_passive = models.TextField(default='[]', verbose_name='Passive fighting history', help_text="Fighting history records of the team in JSON format.  If this is to be modified on django admin site, please make sure it retains valid in JSON format.")
    def __str__(self):
        return self.teamname

    def get_member_limit(self):
        return MEMBER_ALLOWED

    def get_member(self):
        return json.loads(self.members)

    def add_member(self, newMember):
        memberList = json.loads(self.members)
        if len(memberList) < MEMBER_ALLOWED:
            memberList.append(newMember)
            self.member_num += 1
        else:
            pass
        self.members = json.dumps(memberList)
        self.save()

    def delete_member(self, memberToBeDel):
        memberList = json.loads(self.members)
        if memberToBeDel == self.captain:
            return None
        if memberToBeDel in memberList:
            memberList.remove(memberToBeDel)
            self.member_num -= 1
        self.members = json.dumps(memberList)
        self.save()

    def get_history_active(self):
        return json.loads(self.history_active)

    def add_history_active(self, x):
        a = json.loads(self.history_active)
        a.append(x)
        if len(a) > HISTORY_LENGTH:
            del a[0]
        else:
            pass
        self.history_active = json.dumps(a)
        self.save()

    def get_history_passive(self):
        return json.loads(self.history_passive)

    def add_history_passive(self, x):
        a = json.loads(self.history_passive)
        a.append(x)
        if len(a) > HISTORY_LENGTH:
            del a[0]
        else:
            pass
        self.history_passive = json.dumps(a)
        self.save()


    def save(self, *args, **kwargs):
        memberList = json.loads(self.members)
        self.member_num = len(memberList)
        #do something with score
        super().save(*args, **kwargs)

    def memberInTeam(self, targetID):
        if targetID in self.get_member():
            return True
        else:
            return False


    def get_teamInfo(self, showtype=0):
        output = dict()
        output["teamid"] = self.pk
        output["teamname"] = self.teamname
        output["description"] = self.description
        output["captainID"] = self.captain
        output["membersID"] = self.get_member()
        output["createAt"] = self.createAt
        output["score"] = self.score
        output["rank"] = self.rank
        if showtype > 0:
            output["history_active"] = self.get_history_active()
            output["history_passive"] = self.get_history_passive()
            if showtype == 2:
                output["invitecode"] = self.invitecode
        return output


    '''def get_score(self):
        return json.loads(self.score)

    def add_score(self, x):
        a = json.loads(self.score)
        a.append(x)
        if len(a) > HISTORY_LENGTH:
            del a[0]
        else:
            pass
        self.score = json.dumps(a)
        self.save()'''


class Student(models.Model):
    team_name = models.ForeignKey(Team, on_delete=models.CASCADE, null=True)
    is_leader = models.BooleanField(default=False)
    is_active = models.BooleanField(default = False)
    student_id = models.CharField(max_length=20, null=True)
    student_nickname = models.CharField(max_length=50, null=True)
    student_realname = models.CharField(max_length=20,null = True)
    password = models.CharField(max_length=100, default='000', null=True)
    salt = models.CharField(max_length = 8, null = True)
    thu_email = models.CharField(max_length=100, null=True)
    profile_photo = models.ImageField(null=True)

    def __str__(self):
        return self.student_nickname


class Announcement(models.Model):
    title = models.CharField(max_length=20)
    content = models.TextField()
    pub_date = models.DateTimeField(auto_now_add = True)
    last_update_date = models.DateTimeField(auto_now = True)
    def get_AnnouncementInfo(self, showtype=0):
        output = dict()
        output["id"] = self.pk
        output["title"] = self.title
        output["pub_date"] = tzd.localtime(self.pub_date)
        output["last_update_date"] = tzd.localtime(self.last_update_date)
        if showtype > 0:
            output["content"] = self.content
        return output

    def __str__(self):
        return self.title


class File(models.Model):
    title = models.CharField(max_length=50)
    content = models.FileField(upload_to='Files')
    last_update_date = models.DateTimeField(auto_now=True)

    def __str__(self):
        return self.title

    def filename(self):
        return os.path.basename(self.content.file.name)


    def get_FileInfo(self):
        output = dict()
        output["id"] = self.pk
        output["title"] = self.title
        output["last_update_date"] = self.last_update_date
        return output



class GlobalSetting(models.Model):
    year = models.IntegerField()
    game_name = models.CharField(max_length = 50)
    submission_start = models.DateTimeField()
    submission_end = models.DateTimeField()

    def __str__(self):
        return str(self.year)

    def get_globalSetting(self):
        output = dict()
        output["year"] = self.year
        output["game_name"] = self.game_name
        output["submission_start"] = tzd.localtime(self.submission_start)
        output["submission_end"] = tzd.localtime(self.submission_end)
        return output

    def save(self, *args, **kwargs):
        if GlobalSetting.objects.exists() and not self.pk:
            raise ValidationError('Only one Global Settings allowed.')
        return super(GlobalSetting, self).save(*args, **kwargs)




class Battle(models.Model):
    id = models.AutoField(primary_key= True, verbose_name="Battle ID")
    team_engaged   = models.TextField()
    id_map       = models.TextField(null=True)
    initiator_name = models.CharField(max_length=30, null=True)
    request_time   = models.DateTimeField(auto_now_add= True)
    start_time     = models.DateTimeField(null=True)
    robot_num      = models.IntegerField()
    status         = models.IntegerField()
    result         = models.TextField( null=True)
    def __str__(self):
        return self.request_time.strftime("%Y-%m-%d %H:%M:%S") + ':' +   self.team_engaged

