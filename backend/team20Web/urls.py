"""team20Web URL Configuration

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/2.1/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  path('', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  path('', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.urls import include, path
    2. Add a URL to urlpatterns:  path('blog/', include('blog.urls'))
"""
from django.contrib import admin
from django.urls import path, include
from backend import views

urlpatterns = [
    path('api/admin/', admin.site.urls),
    path('api/', views.index, name='index'),
    path('api/announcement/<int:post_id>', views.detail, name='announcement_detail'),
    path('api/auth', views.auth),
    path('api/auth/', views.auth),
    path('api/users', views.users),
    path('api/users/', views.users),
    path('api/users/<int:user_id>', views.modifyUser),
    path('api/teams', views.teams),
    path('api/teams/', views.teams),
    path('api/teams/<int:teamid>', views.modifyTeamByID),
    path('api/teams/<int:teamid>/members', views.modifyTeamMembersByID),
    path('api/announce/list', views.listAnnouncementAPI),
    path('api/announce/view/<int:post_id>', views.viewAnnouncementAPI),
    path('api/file/list', views.listFileAPI),
    path('api/file/download/<int:file_id>', views.downloadFileAPI),
    path('api/global', views.getGlobalSettings),
]
