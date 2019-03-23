from django.urls import path, include
from . import views

urlpatterns = [
    path('add', views.add_battle),
    path('end', views.end_battle),
    path('compile', views.compile),  #暂时写在battle里面
    path('debug', views.debug),
    path('debug/clear', views.clear),
]
