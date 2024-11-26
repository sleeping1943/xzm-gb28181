 #只推送音频 -re 按照输入文件帧率发送 -stream_loop -1 无限循环 -vn 不要视频 -acodec copy 音频使用原格式
 ffmpeg -re -i ".\凯尔莫罕.mp4" -vn -acodec copy -f rtp rtp://10.23.132.27:10000    # 这种推流ZLMediakit会报错
 ffmpeg -re -stream_loop -1 -i ".\凯尔莫罕.mp4" -vn -acodec copy -f rtp_mpegts rtp://10.23.132.27:10000    # 这种推流ZLMediakit会报错
 ffmpeg -re -stream_loop -1 -i ".\1.mp4" -vcodec h264 -acodec copy -f rtp_mpegts rtp://10.23.132.27:10000    # 这种推流ZLMediakit会报错

 # 音频重采样 -ac 1 单通道 -ar 采样率8000hz
 ffmpeg -re -stream_loop -1 -i ".\凯尔莫罕.mp4" -vcodec h264 -ac 1 -ar 8000 -f rtp_mpegts rtp://10.23.132.27:10000    # 这种推流ZLMediakit会报错

# 只推流音频的国标格式
ffmpeg.exe -re -stream_loop -1 -i "./1.mp4" -vn -acodec aac -f rtp_mpegts rtp://10.23.132.27:10000

# 推流g711的国标音频格式
ffmpeg -re -stream_loop -1 -i "./1.mp4" -vn -acodec pcm_alaw -ac 1 -ar 8000 -f rtsp rtsp://10.23.132.27:554/rtp/talk_01

# ffmpeg在windows上命令查询输入输出设备
ffmpeg -list_devices true -f dshow -i dummy 

# ffmpeg采集音频
ffmpeg -f dshow -i audio="麦克风 (USB 2.0 Camera)" -acodec aac -f rtp_mpegts rtp://10.23.132.27:10000
ffmpeg -f dshow -i audio="麦克风 (USB 2.0 Camera)" -f rtp_mpegts rtp://10.23.132.27:10000
# ffmpeg采集音频推流 输出参数: 采样率-8kHz 编码率-64kb/s
# 推流rtp OK
ffmpeg -f dshow -i audio="麦克风 (USB 2.0 Camera)" -acodec aac -ac 1 -ar 8000 -ab 64000 -f rtp_mpegts rtp://10.23.132.27:10000
# 推流rtsp OK
ffmpeg -f dshow -i audio="麦克风 (USB 2.0 Camera)" -acodec aac -ac 1 -ar 8000 -ab 64000 -f rtsp rtsp://10.23.132.27:554:/xzm/test01
# 推流国标流 rtsp,ZLMediakit显示: PCMA[8000/1/16] OK
ffmpeg -f dshow -i audio="麦克风 (USB 2.0 Camera)" -acodec pcm_alaw -ac 1 -ar 8000 -ab 64000 -f rtsp rtsp://10.23.132.27:554:/xzm/test01
# 推流g711的国标音频格式, ZLMediakit 没有打印,没有流信息 NO
ffmpeg -f dshow -i audio="麦克风 (USB 2.0 Camera)" -acodec pcm_alaw -ac 1 -ar 8000 -ab 64000 -f rtp_mpegts rtp://10.23.132.27:10000
#  推流rtp音频流到ZLMediakit PCMA[8000Hz 1 channels s16 64kb/s] OK
ffmpeg -f dshow -i audio="麦克风 (USB 2.0 Camera)" -acodec pcm_alaw -ac 1 -ar 8000 -ab 64000  -f rtp rtp://10.23.132.27:10000