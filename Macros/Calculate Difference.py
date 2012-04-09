# /*******************************************************************
# Calculate Difference ParaView Macro
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
	File -> Open -> open a second dataset
		(same variables and geometry)
		Convert to point data if necessary.
	Select both datasets by Ctrl-clicking.
        Tools -> Python Shell
	Macros -> "Calculate Difference"
"""
#_____________________________________________________________________
try:
	from PyQt4 import QtGui, QtCore
	# QT GUI FUNCTION DEFINITIONS
	def getItem(answers, question):
		## answers must be list of strings
		(ret, ok) = QtGui.QInputDialog.getItem(
			QtGui.QWidget(), question,
			question, answers, 0, True)
		if not ok:
			raise Exception('select error')
		return str(ret)
	def alert(msg):
		QtGui.QMessageBox(QtGui.QMessageBox.Information,
			'Alert', msg).exec_()
except Exception as e:
	#print 'PyQt4 import failed. Falling back on console functions.'
	# CONSOLE FUNCTION DEFINITIONS
	def getItem(answers, question):
		lookup = {}
		print question
		for i in xrange(len(answers)):
			lookup[answers[i].strip()] = i + 1
			print '%3i) %s' % (i+1,answers[i])
		while True:
			try:
				input = raw_input('? ').strip()
			except EOFError:
				raise Exception('no input')
			if input in lookup:
				print ''
				return answers[lookup[input] - 1]
			try:
				input = int(input)
			except ValueError:
				input = -1
			if (input >= 1) and (input <= len(answers)):
				print ''
				return answers[input - 1]
			print "Try again."
	def alert(msg):
		print '\n%s\n' % msg
#_____________________________________________________________________

def calc_diff(data):
	if len(data) != 2:
		alert('Please select two sources before running this script.')
		return None

	data[0].UpdatePipeline()
	data[1].UpdatePipeline()

	if ((len(data[0].PointData.keys()) == 0)
			or (len(data[1].PointData.keys()) == 0)):
		alert('Please convert to point data before running this script.')
		return None

	keylist = [ key for key in data[0].PointData.keys()
				if key in data[1].PointData.keys() ]

	if len(keylist) == 0:
		alert('There does not seem to be any common data in these two sets.')
		return None

	try:
		myvar = getItem(keylist, "Which data field do you want to plot?")
	except Exception as e:
		alert("choose something!")
		return None

	print 'plotting data field = \''+myvar+'\''

	diff_name = '%s Difference' % myvar
	diff_name_v = '%s_Difference' % myvar
	pyf = ("inputs[0].PointData['%s'] - inputs[1].PointData['%s']" %
		   (myvar,myvar))

	diff = PythonCalculator( guiName=diff_name, Expression=pyf,
		ArrayName=diff_name_v,ArrayAssociation='Point Data',
		CopyArrays=1, Input=data )

	SetActiveSource(diff)
	diff.UpdatePipeline()
	return diff

calc_diff(active_objects.get_selected_sources())
