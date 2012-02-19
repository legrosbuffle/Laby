# -*- coding: utf-8 -*-
import Blender
import math

obj = Blender.Object.New('Empty' , 'RingMovement')
Blender.Scene.GetCurrent().link(obj)

lipo = Blender.Ipo.New('Object','RingIpo')

obj.setIpo(lipo)

posCurveX = lipo.addCurve('LocX')
posCurveY = lipo.addCurve('LocY')

hf = open("out.path", "r")

for line in hf:
	#line is:
	#time topX topY bottomX bottomY ringX ringY
	zplit = line.split()
	time = int(zplit[0])
	posX = float(zplit[1])
	posY = float(zplit[2])
	posCurveX.addBezier ((10*time + 1, posX))
	posCurveY.addBezier ((10*time + 1, posY))

hf.close()

posCurveX.setInterpolation('Linear')
posCurveX.Recalc()
posCurveY.setInterpolation('Linear')
posCurveY.Recalc()

Blender.Redraw()