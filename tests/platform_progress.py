#!/usr/bin/env python3
"""File/account contracts, persistent rootfs, and legacy volume migration in QEMU."""
import argparse, hashlib, os, selectors, shutil, subprocess, sys, tempfile, time
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--qemu',required=True);p.add_argument('--kernel',required=True);p.add_argument('--image',required=True);p.add_argument('--mkfs',required=True);p.add_argument('--cpus',type=int,default=5);p.add_argument('--persistent',action='store_true');a=p.parse_args()
ROOT=Path(__file__).resolve().parents[1]
PASSWORD='root-progress-42'; USERPASS='alice-progress-42'; NEWPASS='alice-new-secret-42'
class Guest:
    def __init__(self, image, data=None):
        command=[a.qemu,'-machine','virt','-bios','none','-kernel',a.kernel,'-m','128M','-smp',str(a.cpus),'-nographic','-global','virtio-mmio.force-legacy=false','-drive',f'file={image},if=none,format=raw,id=x0','-device','virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0']
        if data:command+=['-drive',f'file={data},if=none,format=raw,cache=writeback,id=x1','-device','virtio-blk-device,drive=x1,bus=virtio-mmio-bus.1']
        self.process=subprocess.Popen(command,stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,bufsize=0)
        self.selector=selectors.DefaultSelector();self.selector.register(self.process.stdout,selectors.EVENT_READ);self.text=bytearray();self.cursor=0
    def send(self,text):self.process.stdin.write(text if isinstance(text,bytes) else (text+'\n').encode());self.process.stdin.flush()
    def expect(self,token,timeout=45):
        token=token.encode();deadline=time.monotonic()+timeout
        while time.monotonic()<deadline:
            found=self.text.find(token,self.cursor)
            if found>=0:self.cursor=found+len(token);return
            if self.process.poll() is not None:raise RuntimeError('QEMU exited')
            for key,_ in self.selector.select(.1):
                data=os.read(key.fd,65536);self.text.extend(data)
                if b'panic:' in self.text or b'storagecheck: FAIL:' in self.text:raise RuntimeError('guest failure')
        raise TimeoutError('waiting for '+repr(token))
    def login(self,first=False,user='root',password=PASSWORD):
        if first:
            self.expect('First boot: set the root password');self.expect('New password: ');self.send(PASSWORD);self.expect('Retype password: ');self.send(PASSWORD)
        self.expect('login: ');self.send(user);self.expect('Password: ');self.send(password);self.expect('$ ')
    def run(self,text,result=None):
        self.send(text);self.expect(text+'\n')
        if result:self.expect(result)
        self.expect('$ ')
    def password(self,user,password):
        self.send('passwd '+user);self.expect('New password: ');self.send(password);self.expect('Retype password: ');self.send(password);self.expect('passwd: password updated');self.expect('$ ')
    def close(self,label):
        try:self.send(b'\x01x');self.process.wait(timeout=5)
        except Exception:self.process.kill();self.process.wait()
        self.selector.close();(Path(a.image).parent/f'{label}-{a.cpus}cpu.log').write_bytes(self.text)
        for secret in [PASSWORD,USERPASS,NEWPASS]:assert secret.encode() not in self.text,'password echoed'
with tempfile.TemporaryDirectory(prefix='xv-progress-',dir=Path(a.image).parent) as temporary:
    root=Path(temporary);image=root/'fs.img';data=root/'data.img';seed=root/'seed.img';shutil.copyfile(a.image,seed)
    if a.persistent:
        subprocess.run(['python3',str(ROOT/'tools/create_rootfs.py'),'--seed',str(seed),'--output',str(image)],check=True)
        subprocess.run(['python3',str(ROOT/'tools/create_data_image.py'),'--mkfs',a.mkfs,'--output',str(data)],check=True)
    else:shutil.copyfile(seed,image)
    g=Guest(image,data if a.persistent else None)
    try:
        g.login(first=True)
        if a.cpus>1:
            for cpu in range(1,a.cpus):assert f'hart {cpu} starting'.encode() in g.text
        home='/root'
        g.run('pwd',home+'\n')
        if a.persistent:
            g.run('mount','xv6 disk filesystem on /\n')
            g.run('echo $HOME','/root\n')
            g.run('echo PERSISTENT_CONTENT > /root/retained')
            g.run('useradd alice','uid=1000');g.password('alice',USERPASS)
            g.run('groupadd staff','groupadd: done');g.run('usermod -aG staff alice')
            g.run('sync')
        else:
            g.run('storagecheck','storagecheck: all tests succeeded')
            g.run('vmcheck','vmcheck: regions, COW, copyout, lazy pages and protection passed')
            if b'physical allocator: buddy' in g.text:
                g.run('superpgfork','superpgfork: all tests succeeded')
            g.run('no_such_command || echo OR_WORKS','OR_WORKS\n')
            g.run('echo $?','0\n')
            g.run('no_such_command && echo MUST_SKIP')
            g.run('echo $?','127\n')
            g.run('jobs && echo JOB_STATUS_OK','JOB_STATUS_OK\n')
            g.run('echo (','syntax');g.run('echo $?','2\n')
            g.run('cd /tmp && pwd','/tmp\n')
            g.run("echo 'echo SCRIPT_FIRST' > /tmp/script")
            g.run("echo 'missing || echo SCRIPT_OR' >> /tmp/script")
            g.run("echo 'echo $?; cd /root; pwd' >> /tmp/script")
            g.run('sh /tmp/script','SCRIPT_OR\n');g.run('cat /tmp/script','echo SCRIPT_FIRST')
            g.run("echo 'cat; echo SCRIPT_AFTER' > /tmp/readscript")
            g.send('sh /tmp/readscript');g.expect('sh /tmp/readscript\n');g.send('SCRIPT_INPUT');g.expect('SCRIPT_INPUT\n');g.expect('SCRIPT_INPUT\n');g.send(b'\x04');g.expect('SCRIPT_AFTER\n');g.expect('$ ')
            g.run("echo 'exit 7; echo MUST_NOT_RUN' > /tmp/exit-script")
            g.run('sh /tmp/exit-script');g.run('echo $?','7\n')
            g.run('cd /');g.run('useradd alice','uid=1000');g.password('alice',USERPASS)
            g.run('groupadd staff','groupadd: done');g.run('usermod -aG staff alice')
            g.run('su alice id','groups=1000,1001')
            g.run('su alice');g.send('passwd');g.expect('Current password: ');g.send(USERPASS);g.expect('New password: ');g.send(NEWPASS);g.expect('Retype password: ');g.send(NEWPASS);g.expect('passwd: password updated');g.expect('$ ')
            g.run('id','euid=1000');g.run('passwd root','passwd: permission denied')
            g.send(b'\x04');g.expect('$ ')
            g.send(b'\x04');g.login(user='alice',password=NEWPASS);g.run('id','groups=1000,1001')
            g.send(b'\x04');g.login();g.run('userdel alice','userdel: done');g.run('useradd bob','uid=1002')
            g.run('groupdel staff','groupdel: done')
    finally:g.close('persistent-first' if a.persistent else 'platform-progress')
    if a.persistent:
        # Rebuilding the installation seed must not replace the live rootfs.
        before=image.read_bytes();shutil.copyfile(a.image,seed)
        subprocess.run(['python3',str(ROOT/'tools/create_rootfs.py'),'--seed',str(seed),'--output',str(image)],check=True)
        assert image.read_bytes()==before,'rootfs generator overwrote the existing root disk'
        g=Guest(image,data)
        try:
            g.login();assert b'First boot:' not in g.text
            g.run('pwd','/root\n');g.run('cat /root/retained','PERSISTENT_CONTENT\n')
            g.run('su alice id','groups=1000,1001');g.run('su alice pwd','/home/alice\n')
            g.run('umount / || echo ROOT_PROTECTED','ROOT_PROTECTED\n')
            # Optional secondary disks mount explicitly and never relocate HOME/accounts.
            g.run('mount -t xv6 /data');g.run('echo AUXILIARY > /data/retained')
            g.run('echo $HOME','/root\n');g.run('su alice id','groups=1000,1001')
            g.run('umount /data');g.run('mount -t xv6 /data');g.run('cat /data/retained','AUXILIARY\n')
        finally:g.close('persistent-reboot')

        # Import a private legacy /data volume: credentials, ownership, links and large files.
        sys.path.insert(0,str(ROOT/'tools'))
        from create_rootfs import Image, DIRECTORY, FILE, SYMLINK
        legacy=root/'legacy.img';subprocess.run(['python3',str(ROOT/'tools/create_data_image.py'),'--mkfs',a.mkfs,'--output',str(legacy)],check=True)
        volume=Image(legacy);entries=volume.entries(1);etc=entries[b'etc'];home=entries[b'home'];rhome=entries[b'root']
        def add(parent,name,content,attributes=(0,0,0o644),kind=FILE):
            inode=volume.create(parent,name,kind,attributes);volume.append(inode,content);return inode
        add(etc,b'passwd',b'root:!:0:0:root:/data/root:/bin/sh\nalice:!:1000:1000:alice:/data/home/alice:/bin/sh\n')
        salt=bytes(range(16));digest=hashlib.pbkdf2_hmac('sha256',PASSWORD.encode(),salt,100000)
        add(etc,b'shadow',('root:pbkdf2-sha256$100000$'+salt.hex()+'$'+digest.hex()+'\n').encode(),(0,0,0o600))
        add(etc,b'group',b'root:!:0:\nalice:!:1000:\nstaff:!:1001:alice\n')
        ahome=volume.create(home,b'alice',DIRECTORY,(1000,1000,0o750))
        note=add(ahome,b'note',b'IMPORTED_OWNER\n',(1000,1000,0o640));volume.link(ahome,b'alias',note)
        add(ahome,b'link',b'note',(1000,1000,0o777),SYMLINK)
        add(rhome,b'big',b'X'*20480);add(rhome,b'retained',b'IMPORTED_ROOT\n')
        legacy.write_bytes(volume.data);old=legacy.read_bytes();migrated=root/'migrated.img'
        subprocess.run(['python3',str(ROOT/'tools/create_rootfs.py'),'--seed',str(seed),'--output',str(migrated),'--import-data',str(legacy)],check=True)
        assert legacy.read_bytes()==old,'migration modified the original data disk'
        imported=Image(migrated);newhome=imported.entries(imported.entries(1)[b'home'])[b'alice'];names=imported.entries(newhome)
        assert names[b'note']==names[b'alias'] and imported.attributes(names[b'note'])==(1000,1000,0o640)
        assert imported.kind(names[b'link'])==SYMLINK and imported.read(names[b'link'])==b'note'
        assert imported.read(imported.entries(imported.entries(1)[b'root'])[b'big'])==b'X'*20480
        g=Guest(migrated)
        try:
            g.login();assert b'First boot:' not in g.text
            g.run('pwd','/root\n');g.run('cat retained','IMPORTED_ROOT\n')
            g.run('su alice pwd','/home/alice\n');g.run('su alice cat /home/alice/link','IMPORTED_OWNER\n')
            g.run('su alice id','groups=1000,1001');g.run('cp big /tmp/big');g.run('wc /tmp/big','20480')
        finally:g.close('rootfs-migrated')
print('Platform progress tests passed')
