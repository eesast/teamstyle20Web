from django.db import models

# Create your models here.

class Battle(models.Model):
	battleId = models.IntegerField()
	userId1 = models.IntegerField()
	userId2 = models.IntegerField()
	running = models.BooleanField()

	def __str__(self):
		return 'id:%d  user1:%d  user2:%d  running:'%(self.battleId,self.userId1,self.userId2) + 'True' if self.running else 'False'
