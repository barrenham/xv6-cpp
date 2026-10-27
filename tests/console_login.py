"""Drive the real first-boot enrollment and login flow on isolated test disks."""
PASSWORD = 'xv6-test-root-42'

def login_root(expect, process):
    def send(text):
        process.stdin.write((text + '\n').encode()); process.stdin.flush()
    expect('First boot: set the root password', timeout=30)
    expect('New password: ')
    send(PASSWORD)
    expect('Retype password: ')
    send(PASSWORD)
    expect('login: ', timeout=60)
    send('root')
    expect('Password: ')
    send(PASSWORD)
    expect('root:/root$ ', timeout=60)
    send('cd /')
    expect('root:/$ ')
