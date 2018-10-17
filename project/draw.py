#!/usr/bin/python3
#-- coding: utf8 --

from tkinter import Tk, Canvas, Frame, BOTH
import serial
import threading
import time

class Draw(Frame):
  
    def __init__(self):
        super().__init__()   
         
        self.initUI()
        
        
    def initUI(self):
      
        self.master.title("Lines")        
        self.pack(fill=BOTH, expand=1)

        self.canvas = Canvas(self)
        self.canvas.create_oval(50,440,60,450,fill="blue",width=0)
        self.nowx = 55
        self.nowy = 445
        
        
        self.canvas.bind("<Button-1>", self.newPosition)
        self.canvas.pack(fill=BOTH, expand=1)

    def newPosition(self,event):
        print("clicked at", event.x, event.y)
        self.canvas.create_oval(event.x-5,event.y-5,event.x+5,event.y+5,fill='blue',width=0)

    def parser(self,data):
        return {"x":1,"y":1}

    def recvinfor(self,data):
        info = self.parser(data)
        #(x,y) = (info["x"],info["y"])
        for i in range(10):
            self.nowx += 1
            self.nowy +=1
            x = self.nowx
            y = self.nowy
            self.canvas.create_oval(x-5,y-5,x+5,y+5,fill='blue',width=0)




def loop(ser,mydraw):
    while ser.isOpen():
        if (ser.in_waiting > 0):
            buffer = ser.read(ser.in_waiting)
            print(type(buffer))
            print(buffer)
            mydraw.recvinfor(buffer)
        elif (ser.in_waiting <= 0):
            time.sleep(1)

def main():

    root = Tk()

    bluetooth = serial.Serial("com10",9600,timeout=0)
  
    ex = Draw()
    thread = threading.Thread(target=loop,args=(bluetooth,ex,))
    thread.start()
    root.geometry("800x500+300+100")
    root.mainloop()  

if __name__ == '__main__':
    main()  
