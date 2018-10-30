'''
This file uses tkinter to draw path of our rebot, while communicating via bluetooth.

There are some constants very import, including:
basespeed (define the drawing speed)
degree_per_second (define the turning speed)
port_number (define which port used)
'''

import tkinter
from tkinter import Tk, Canvas, Frame, BOTH, Text, END
import serial
import threading
import time
import math


def d2r(degree):
    #turn degree to rotation
    return degree / 180 * math.pi


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

        self.canvas = Canvas(self)
        self.currentcar = self.canvas.create_oval(
            50, 440, 60, 450, fill="blue", width=0)
        self.infobuffer = ''

        self.nowx = 55
        self.nowy = 445
        self.direction = 270
        self.interval = 0.1
        self.degree_per_second = 90 / 1.160
        self.basespeed = 2

        self.canvas.pack(fill=BOTH, expand=1)

    def newPosition(self, deltax, deltay, color='blue'):
        #print("clicked at", event.x, event.y)
        cor = self.canvas.coords(self.currentcar)
        x0 = (cor[0] + cor[2])/2
        y0 = (cor[1] + cor[3])/2
        step = 5
        for i in range(step):
            dx = (deltax / step) * (i + 1)
            dy = (deltay / step) * (i + 1)
            self.currentcar = self.canvas.create_oval(
                x0 + dx - 5, y0 + dy - 5, x0 + dx + 5, y0 + dy + 5, fill=color, width=0)
            time.sleep(self.interval/step)
            self.canvas.update()

    @staticmethod
    def rendercolor(light, temp):
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
        if direction == 0:
            degree = 0
        elif direction == 1:
            degree = -degree
        
        self.direction = (self.direction + degree) % 360
        dx = math.cos(d2r(self.direction)) * self.basespeed
        dy = math.sin(d2r(self.direction)) * self.basespeed
        if direction != 0:
            return (0,0)
        else:
            return (dx, dy)

    def recvinfor(self, data):
        self.infobuffer += data.decode('utf8')
        buf = self.infobuffer
        newlinepos = buf.find('@\r\n')
        if newlinepos == -1:
            return
        onemessage = buf[:newlinepos]
        self.infobuffer = buf[newlinepos+1:]
        info = self.parser(onemessage)
        if info == None:
            return
        print(info)
        direction = info['direction']
        (dx, dy) = self.move(direction)
        color = Draw.rendercolor(info['light'], info['temp'])
        self.newPosition(dx, dy, color)


def loop(ser, mydraw):
    #This thread communicates via bluetooth
    while ser.isOpen():
        if (ser.in_waiting > 0):
            buffer = ser.read(ser.in_waiting)
            #print(buffer)
            mydraw.recvinfor(buffer)
        elif (ser.in_waiting <= 0):
            time.sleep(0.02)


def main():
    #setup
    bluetooth = serial.Serial("com11",9600,timeout=0)
    root = Tk()
    ex = Draw(root)

    #start a thread to use bluetooth async
    thread = threading.Thread(target=loop,args=(bluetooth,ex,))
    thread.start()

    #start gui
    root.geometry("800x500+300+100")
    root.mainloop()

if __name__ == '__main__':
    main()
