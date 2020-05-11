
### 1 Run socat 
Open new terminal tab and run:
socat -d -d pty pty

### 2 Run xlgyro_fake
Take PTY from socat

Open new terminal tab and run:
/xlgyrod --tty PTY(output)

### 3 Run xlgyro
Open new terminal tab and run:
./xlgyro_fake_input PTY(input)



