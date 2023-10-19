 #只推送音频 -re 按照输入文件帧率发送 -stream_loop -1 无限循环 -vn 不要视频 -acodec copy 音频使用原格式
 ffmpeg -re -i ".\凯尔莫罕.mp4" -vn -acodec copy -f rtp rtp://10.23.132.27:10000    # 这种推流ZLMediakit会报错
 ffmpeg -re -stream_loop -1 -i ".\凯尔莫罕.mp4" -vn -acodec copy -f rtp_mpegts rtp://10.23.132.27:10000    # 这种推流ZLMediakit会报错
 ffmpeg -re -stream_loop -1 -i ".\1.mp4" -vcodec h264 -acodec copy -f rtp_mpegts rtp://10.23.132.27:10000    # 这种推流ZLMediakit会报错

 # 音频重采样 -ac 1 单通道 -ar 采样率8000hz
 ffmpeg -re -stream_loop -1 -i ".\凯尔莫罕.mp4" -vcodec h264 -ac 1 -ar 8000 -f rtp_mpegts rtp://10.23.132.27:10000    # 这种推流ZLMediakit会报错

# 只推流音频的国标格式
ffmpeg.exe -re -stream_loop -1 -i "./1.mp4" -vn -acodec aac -f rtp_mpegts rtp://10.23.132.27:10000