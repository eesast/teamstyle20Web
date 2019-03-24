from django.contrib import admin
from .models import *

# Register your models here.
admin.site.site_url = '/api/'

class GlobalSettingAdmin(admin.ModelAdmin):
    list_display = ('year', 'game_name', 'submission_start', 'submission_end')
    list_display_links = ('year', 'game_name',)
    def has_add_permission(self, request):
        if self.model.objects.count() > 0:
            return False
        else:
            return True
    def has_delete_permission(self, request, obj=None):
        return False
class AnnouncementAdmin(admin.ModelAdmin):
    list_display = ('title', 'pub_date', 'last_update_date')
    readonly_fields = ('last_update_date',)

class TeamAdmin(admin.ModelAdmin):
    list_display = ('pk','teamname','captain','members', 'member_num','createAt')
    readonly_fields = ('pk','member_num', 'createAt',)
    list_display_links = ('pk', 'teamname')
class FileAdmin(admin.ModelAdmin):
    list_display = ('pk', 'title', 'last_update_date')
    list_display_links = ('pk','title', )




class BattleAdmin(admin.ModelAdmin):
    list_display = ('id', 'team_engaged', 'request_time', 'status', 'robot_num','result')
    list_display_links = ('id', 'team_engaged', 'request_time')
    readonly_fields = ('id','team_engaged', 'start_time','request_time','robot_num', 'status', 'result', 'id_map', 'initiator_name')
    def has_delete_permission(self, request, obj=None):
        return False
    def save_model(self, request, obj, form, change):
        pass
    def has_add_permission(self, request):
        return False

#admin.site.register(Student)
admin.site.register(Team, TeamAdmin)
admin.site.register(Announcement, AnnouncementAdmin)
admin.site.register(File, FileAdmin)
admin.site.register(GlobalSetting, GlobalSettingAdmin)
admin.site.register(Battle,BattleAdmin)


