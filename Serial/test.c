#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>


int serial_open(SerialPort *port, const char *device) 
{
    port->fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    if (port->fd == -1) 
	{
        perror("Unable to open port");
        return -1;
    }

    struct termios options;
    tcgetattr(port->fd, &options);
    cfsetispeed(&options, B9600);  // Set baud rate to 9600 (adjust as needed)
    cfsetospeed(&options, B9600);

    options.c_cflag &= ~PARENB;  // No parity
    options.c_cflag &= ~CSTOPB;  // One stop bit
    options.c_cflag &= ~CSIZE;   // Mask the character size bits
    options.c_cflag |= CS8;      // 8 data bits

    tcsetattr(port->fd, TCSANOW, &options);

    return 0;
}

void serial_close(SerialPort *port)
{
    close(port->fd);
    port->fd = -1;
}

int serial_read(SerialPort *port, char *buffer, size_t size)
{
    return read(port->fd, buffer, size);
}




int main()
{
    SerialPort port;
    if (serial_open(&port, "/dev/ttyACM0") == -1)
	{
        return -1;
    }

    char buffer[256];
    int bytesRead;

    while (1) 
	{
        bytesRead = serial_read(&port, buffer, sizeof(buffer)-1);

        if (bytesRead > 0) 
		{
            buffer[bytesRead] = '\0';
            printf("%s", buffer);
			
        }
    }

    serial_close(&port);
    return 0;
}