#!/usr/bin/env python3
"""Verify the real VirtIO-GPU scanout via QMP, without a desktop session."""
import argparse,json,os,selectors,shutil,socket,struct,subprocess,tempfile,time
from pathlib import Path
from console_login import login_root
p=argparse.ArgumentParser();p.add_argument('--qemu',required=True);p.add_argument('--kernel',required=True);p.add_argument('--image',required=True);p.add_argument('--cpus',type=int,default=5);p.add_argument('--without-gpu',action='store_true');p.add_argument('--vnc',action='store_true');p.add_argument('--width',type=int,default=1280);p.add_argument('--height',type=int,default=800);a=p.parse_args()
if a.vnc and a.without_gpu:p.error('--vnc requires a GPU')

def receive(stream,count):
 data=bytearray()
 while len(data)<count:
  part=stream.recv(count-len(data))
  if not part:raise RuntimeError('VNC disconnected')
  data.extend(part)
 return bytes(data)

def vnc_pixels(port):
 # A minimal independent RFB 3.8 client, requesting raw BGRX pixels.
 with socket.create_connection(('127.0.0.1',port),timeout=10) as stream:
  assert receive(stream,12)==b'RFB 003.008\n'
  stream.sendall(b'RFB 003.008\n')
  count=receive(stream,1)[0];security=receive(stream,count)
  assert 1 in security,'loopback VNC security negotiation'
  stream.sendall(b'\x01');assert receive(stream,4)==b'\0\0\0\0'
  stream.sendall(b'\x01');header=receive(stream,24)
  width,height=struct.unpack('!HH',header[:4]);name_size=struct.unpack('!I',header[20:])[0]
  assert name_size<65536;receive(stream,name_size)
  assert (width,height)==(a.width,a.height),(width,height)
  pixel_format=struct.pack('!BBBBHHHBBB3x',32,24,0,1,255,255,255,16,8,0)
  stream.sendall(b'\0\0\0\0'+pixel_format)
  stream.sendall(struct.pack('!BBHi',2,0,1,0)) # only raw encoding
  stream.sendall(struct.pack('!BBHHHH',3,0,0,0,width,height))
  pixels=bytearray(width*height*4);covered=0
  for _ in range(32):
   message=receive(stream,1)[0]
   if message==2:continue # bell
   if message==3:
    length=struct.unpack('!I',receive(stream,7)[3:])[0]
    assert length<1048576;receive(stream,length);continue
   assert message==0,('unexpected VNC message',message)
   rectangles=struct.unpack('!H',receive(stream,3)[1:])[0]
   for _ in range(rectangles):
    x,y,w,h,encoding=struct.unpack('!HHHHi',receive(stream,12))
    assert encoding==0 and x+w<=width and y+h<=height,(x,y,w,h,encoding)
    raw=receive(stream,w*h*4)
    for row in range(h):
     start=((y+row)*width+x)*4;pixels[start:start+w*4]=raw[row*w*4:(row+1)*w*4]
    covered+=w*h
   if covered==width*height:return pixels
  raise RuntimeError('incomplete VNC framebuffer')
output=Path(a.image).parent;transcript=bytearray();cursor=0
with tempfile.TemporaryDirectory(prefix='xv-gpu-',dir=output) as directory:
 directory=Path(directory);disk=directory/'fs.img';shutil.copyfile(a.image,disk);monitor=directory/'qmp'
 console=['-nographic','-display','none'] if a.vnc else ['-display','none','-serial','stdio']
 command=[a.qemu,'-machine','virt','-bios','none','-kernel',a.kernel,'-m','128M','-smp',str(a.cpus),*console,'-qmp',f'unix:{monitor},server=on,wait=off','-global','virtio-mmio.force-legacy=false','-drive',f'file={disk},if=none,format=raw,id=x0','-device','virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0']
 if not a.without_gpu:command+=['-device',f'virtio-gpu-device,bus=virtio-mmio-bus.3,xres={a.width},yres={a.height}']
 if a.vnc:
  with socket.socket() as reservation:
   reservation.bind(('127.0.0.1',0));vnc_port=reservation.getsockname()[1]
  assert vnc_port>=5900
  command+=['-vnc',f'127.0.0.1:{vnc_port-5900}']
 guest=subprocess.Popen(command,stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,bufsize=0)
 selector=selectors.DefaultSelector();selector.register(guest.stdout,selectors.EVENT_READ)
 def expect(token,timeout=45):
  global cursor
  token=token.encode();deadline=time.monotonic()+timeout
  while time.monotonic()<deadline:
   found=transcript.find(token,cursor)
   if found>=0:cursor=found+len(token);return
   if guest.poll() is not None:raise RuntimeError('QEMU exited')
   for key,_ in selector.select(.1):transcript.extend(os.read(key.fd,65536))
   if b'panic:' in transcript:raise RuntimeError('kernel panic')
  raise TimeoutError(f'waiting for {token!r}')
 try:
  login_root(expect,guest)
  guest.stdin.write(b'fbdemo\n');guest.stdin.flush()
  if a.without_gpu:expect('fbdemo: no framebuffer');expect('$ ')
  else:
   expect(f'fbdemo: {a.width}x{a.height} RGB bars displayed',timeout=90);expect('$ ')
   with socket.socket(socket.AF_UNIX) as connection:
    connection.settimeout(10);connection.connect(str(monitor));stream=connection.makefile('rwb',buffering=0)
    json.loads(stream.readline())
    def qmp(execute,arguments=None):
     message={'execute':execute}
     if arguments is not None:message['arguments']=arguments
     stream.write(json.dumps(message).encode()+b'\n')
     while True:
      reply=json.loads(stream.readline())
      if 'error'in reply:raise RuntimeError(reply)
      if 'return'in reply:return reply['return']
    qmp('qmp_capabilities')
    if a.vnc:
     config=qmp('query-vnc')
     assert config['enabled'] and config['host']=='127.0.0.1' and config['service']==str(vnc_port),config
    screenshot=directory/'frame.ppm';qmp('screendump',{'filename':str(screenshot)})
   # QEMU emits P6 with a three-line header. Compare actual scanout pixels.
   with screenshot.open('rb') as image:
    assert image.readline().strip()==b'P6';width,height=map(int,image.readline().split());assert image.readline().strip()==b'255';pixels=image.read()
   assert (width,height)==(a.width,a.height) and len(pixels)==a.width*a.height*3,(width,height,len(pixels))
   for y in (0,a.height//2,a.height-1):
    for x,expected in ((0,b'\xff\0\0'),(a.width//3-1,b'\xff\0\0'),(a.width//3,b'\0\xff\0'),(2*a.width//3-1,b'\0\xff\0'),(2*a.width//3,b'\0\0\xff'),(a.width-1,b'\0\0\xff')):
     offset=(y*a.width+x)*3;assert pixels[offset:offset+3]==expected,(x,y,pixels[offset:offset+3])
   if a.vnc:
    remote=vnc_pixels(vnc_port)
    # Verify every pixel delivered through the actual TCP VNC connection.
    for offset in range(a.width*a.height):
     assert remote[offset*4:offset*4+3]==pixels[offset*3:offset*3+3][::-1],offset
   shutil.copyfile(screenshot,output/f'gpu-{a.cpus}cpu.ppm')
  print(f'GPU {a.cpus} CPU: '+('absence handled' if a.without_gpu else f'actual {a.width}x{a.height} QMP scanout pixels verified')+('; loopback VNC handshake and all RGB pixels verified' if a.vnc else ''))
 except Exception:
  print(transcript.decode(errors='replace'));raise
 finally:
  guest.terminate()
  try:guest.wait(timeout=5)
  except subprocess.TimeoutExpired:guest.kill();guest.wait()
  selector.close();(output/('gpu-'+str(a.cpus)+'cpu'+('-absent'if a.without_gpu else'-vnc'if a.vnc else'')+'.log')).write_bytes(transcript)
