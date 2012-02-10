NUMBER_OF_CYCLES = 8
LOW_COLOR = [0xdd, 0xdd, 0xdd]
HIGH_COLOR = [0xcc, 0xbb, 0xbb]
# /*******************************************************************
# Banded Levels Colormap ParaView Macro
# Copyright 2011-2012, The University of North Carolina at Chapel Hill.
# 
# This software was written in 2011 by Hal Canary <hal@cs.unc.edu>
# while working for the MADAI project <http://madai.us/>.
# 
# See copyright.txt for more information.
# *******************************************************************/
"""
How to use:
	Run ParaView.
	File -> Open -> open a dataset
	Macros -> "Banded Levels Colormap"
"""
#_____________________________________________________________________
try:
	from PyQt4 import QtGui, QtCore
	def alert(msg):
		QtGui.QMessageBox(QtGui.QMessageBox.Information,
			'Alert', msg).exec_()
except Exception as e:
	def alert(msg):
		print '\n%s\n' % msg
#_____________________________________________________________________
def makeBands(NumCycles, lowColor, highColor):
	src = active_objects.get_source()
	if src is None:
		alert('Select a Source on the Pipline before running.')
		return
	array = Show(src).ColorArrayName
	if array is None:
		alert('Source lacks a selected array.')
		return
	(mini,maxa) = src.PointData[array].GetRange()
	def colorIntToFloat(colorArray):
		return map(lambda x: (x / 255.0), colorArray)
	lowColor = colorIntToFloat(lowColor)
	highColor = colorIntToFloat(highColor)
	rgb = []
	dist = float(maxa - mini) / NumCycles
	small_dist = dist / 256.0
	for i in xrange(NumCycles):
		rgb.append(mini)
		rgb.extend(lowColor)
		mini += dist
		rgb.append(mini - small_dist)
		rgb.extend(highColor)
	table = GetLookupTableForArray(array, 1)
	table.ColorSpace = 'Lab'
	table.RGBPoints = rgb
	table.NumberOfTableValues = 256
	Render()
makeBands(NUMBER_OF_CYCLES,LOW_COLOR,HIGH_COLOR)

