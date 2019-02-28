from django.contrib import admin
from .models import *

# Register your models here.
class GlobalSettingAdmin(admin.ModelAdmin):
    list_display = ('year', 'game_name', 'submission_start', 'submission_end')
    def has_add_permission(self, request):
        if self.model.objects.count() > 0:
            return False
        else:
            return True
    def has_delete_permission(self, request, obj=None):
        return False
class AnnouncementAdmin(admin.ModelAdmin):
    list_display = ('title', 'pub_date', 'last_update_date')

class TeamAdmin(admin.ModelAdmin):
    list_display = ('teamname','captain','members', 'member_num','createAt')
    readonly_fields = ('member_num', 'createAt',)


#admin.site.register(Student)
admin.site.register(Team, TeamAdmin)
admin.site.register(Announcement, AnnouncementAdmin)
admin.site.register(File)
admin.site.register(GlobalSetting, GlobalSettingAdmin)


