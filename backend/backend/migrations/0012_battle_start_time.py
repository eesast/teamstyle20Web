# Generated by Django 2.1 on 2019-03-13 08:33

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('backend', '0011_auto_20190313_1619'),
    ]

    operations = [
        migrations.AddField(
            model_name='battle',
            name='start_time',
            field=models.DateTimeField(null=True),
        ),
    ]
