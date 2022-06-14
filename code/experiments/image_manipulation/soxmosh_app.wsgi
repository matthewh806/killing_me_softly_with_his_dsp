import sys

path = '/var/www/webapps/image_manipulation/'
if path not in sys.path:
	sys.path.insert(0, path) 

from app import app as application
