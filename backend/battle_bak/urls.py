from django.urls import path, include
from . import views

urlpatterns = [
    path('add', views.add_battle),
    path('end', views.end_battle),
    path('compile', views.compile),  #暂时写在battle里面
#    path('debug/view/queue', views.debug_view_queue),
    path('debug/view/team', views.debug_view_team),
#    path('debug/view/battle', views.debug_view_battle),
#    path('debug/clear/queue', views.debug_clear_queue),
#    path('debug/clear/team', views.debug_clear_team),
#    path('debug/clear/battle', views.debug_clear_battle),
#    path('debug/add/team', views.debug_add_team),
    path('result', views.view_result),
]
