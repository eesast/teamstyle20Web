from django.db import models

# Create your models here.

class Room(models.Model):
    battle_id = models.IntegerField()
    room_id = models.IntegerField()
    key = models.CharField(max_length=130, default='SOME STRING')

    def __str__(self):
        return 'room:%d  battle_id:%d  key:%s'%(self.room_id,self.battle_id,self.key)

class Queue(models.Model):
    battle_id = models.IntegerField()

    def __str__(self):
        return 'battle_id:%d'%(self.battle_id)
