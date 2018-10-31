'''
This file uses tkinter to draw path of our rebot, while communicating via bluetooth.

There are some constants very import, including:
basespeed (define the drawing speed)
degree_per_second (define the turning speed)
port_number (define which port used)
'''

import tkinter
from tkinter import Tk, Canvas, Frame, BOTH, Text, END, Button
#import tkFont
import serial
import threading
import time
import math


#before all functions because key events used it
bluetooth = serial.Serial("com12",9600,timeout=0)

def d2r(degree):
    #turn degree to rotation
    return degree / 180 * math.pi


def leftArrow(event):
    print("Left Arrow")
    print(bluetooth.write(b'\x01'))

def rightArrow(event):
    print("Right Arrow")
    bluetooth.write(b'\x02')

def upArrow(event):
    print("Up Arrow")
    bluetooth.write(b'\x00')

def downArrow(event):
    print("Down Arrow")
    bluetooth.write(b'\x03')

class Draw(Frame):
    '''
    Draw class for tkinter
    '''
    def __init__(self,root):
        super().__init__()

        self.initUI(root)

    def initUI(self,root):
        self.master.title("Auto Detected Robot")
        self.pack(fill=BOTH, expand=1)

        #some basic setting
        self.nowx = 600
        self.nowy = 545
        self.direction = 270
        self.interval = 0.1
        self.degree_per_second = 90 / 1.160
        self.basespeed = 2
        self.currentspeed = 0
        self.infobuffer = ''

        #setup canvas
        self.canvas = Canvas(self)
        self.currentcar = self.canvas.create_oval(
            self.nowx-5, self.nowy-5, self.nowx+5, self.nowy+5, fill="blue", width=0)
        self.temptext = self.canvas.create_text(800,100,text="Temp:",font=("Times", "24"))
        self.lightext = self.canvas.create_text(800,150,text="Light:",font=("Times", "24"))

        self.canvas.pack(fill=BOTH, expand=1)
        '''
        self.left = Button(self.canvas,text='left',font=("","16"))
        self.right = Button(self.canvas,text='right',font=("","16"))
        self.left.pack(side="top", expand=False, anchor='w',padx=4, pady=4)
        self.right.pack(side="top", expand=False, anchor='w',padx=4, pady=4)
        '''

        #bind events
        root.bind('<Left>',leftArrow)
        root.bind('<Right>',rightArrow)
        root.bind('<Up>',upArrow)
        root.bind('<Down>',downArrow)

    def newPosition(self, deltax, deltay, color='blue'):
        # paint new position for the rebot
        cor = self.canvas.coords(self.currentcar)
        x0 = (cor[0] + cor[2])/2
        y0 = (cor[1] + cor[3])/2
        step = 5 # how many points between the previous position
        for i in range(step):
            dx = (deltax / step) * (i + 1)
            dy = (deltay / step) * (i + 1)
            self.currentcar = self.canvas.create_oval(
                x0 + dx - 5, y0 + dy - 5, x0 + dx + 5, y0 + dy + 5, fill=color, width=0)
            time.sleep(self.interval/step)
            self.canvas.update()

    @staticmethod
    def rendercolor(light, temp):
        #change the function to get different color
        return 'blue'

    def parser(self, data):
        sp = data.split()
        #gprint(sp)
        try:
            result = {'direction': int(sp[0]), "light": int(sp[1]), "temp": float(sp[2])}
        except:
            return None
        else:
            return result

    def move(self, direction):
        degree = self.degree_per_second * self.interval

        #check if is turning
        if direction == 0:
            degree = 0
        elif direction == 2:
            degree = -degree

        #check whether stop or not
        if direction == 3:
            self.currentspeed = 0
        else:
            self.currentspeed = self.basespeed
        
        self.direction = (self.direction + degree) % 360
        dx = math.cos(d2r(self.direction)) * self.currentspeed
        dy = math.sin(d2r(self.direction)) * self.currentspeed

        # return deltax and deltay
        if direction != 0:
            return (0,0)
        else:
            return (dx, dy)

    def recvinfor(self, data):
        self.infobuffer += data.decode('utf8',errors='ignore')
        buf = self.infobuffer
        newlinepos = buf.find('@\r\n')

        #if there is not a complete message, then return
        if newlinepos == -1:
            return

        onemessage = buf[:newlinepos]
        self.infobuffer = buf[newlinepos+1:]
        info = self.parser(onemessage)#parse one message
        if info == None:
            return

        print(info)
        direction = info['direction']
        (dx, dy) = self.move(direction)
        color = Draw.rendercolor(info['light'], info['temp'])
        self.canvas.itemconfig(self.temptext,text='Temp: {} °C'.format(info['temp']))
        self.canvas.itemconfig(self.lightext,text='Light: {}'.format(info['light']))
        self.newPosition(dx, dy, color)


def loop(ser, mydraw):
    #This thread communicates via bluetooth
    while ser.isOpen():
        bluetooth.flush()
        time.sleep(0.02)
        if (bluetooth.out_waiting > 0):
            print("ready to write")
        else:
            time.sleep(0.02)
        if (ser.in_waiting > 0):
            buffer = ser.read(ser.in_waiting)
            mydraw.recvinfor(buffer)
        else:
            time.sleep(0.02)


def main():
    #setup
    root = Tk()
    ex = Draw(root)

    #start a thread to use bluetooth async
    thread = threading.Thread(target=loop,args=(bluetooth,ex,))
    thread.start()

    #start gui
    root.geometry("1000x600+300+100")
    root.mainloop()

if __name__ == '__main__':
    main()
