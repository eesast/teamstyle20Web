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
	path('battle/', include('battle.urls')),
    path('admin/', admin.site.urls),
    path('', views.index, name='index'),
    path('announcement/<int:post_id>', views.detail, name='announcement_detail'),
    path('api/auth', views.auth),
    path('api/auth/', views.auth),
    path('api/users', views.users),
    path('api/users/', views.users),
    path('api/users/<int:user_id>', views.modifyUser),
    path('api/teams', views.teams),
    path('api/teams/', views.teams),
    path('api/teams/<int:teamid>', views.modifyTeamByID)
]
