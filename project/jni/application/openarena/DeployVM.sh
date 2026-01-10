#!/bin/sh

scp AndroidData/pak7-android.pk3 user@10.0.0.91:oa/ded1/baseoa

scp AndroidData/pak7-android.pk3 user@10.0.0.92:oa/ded1/baseoa

scp -P 11721 AndroidData/pak7-android.pk3 pelya@45.61.147.135:OpenArena-dedicated-server/baseoa
ssh -p 11721 root@45.61.147.135 shutdown -r now

scp -P 11221 AndroidData/pak7-android.pk3 pelya@185.164.136.111:OpenArena-dedicated-server/baseoa
ssh -p 11221 root@185.164.136.111 shutdown -r now
