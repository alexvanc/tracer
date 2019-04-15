import MySQLdb
import json
import logging
import os


class Configuration(object):
    dirname = os.path.dirname(os.path.realpath(__file__))
    configs = json.loads(open(dirname+"/trace.config").read().replace("\n", ""))
    configs['OutputPath'] = configs['ProjectPath'] + "/output/" + configs['mysql_db']
    # create the output directory
    isExists = os.path.exists(configs['OutputPath'])
    if not isExists:
        os.makedirs(configs['OutputPath'])
    configs['LogPath'] = configs['OutputPath'] + "/trace.log"


class Connect(object):
    def __init__(self):
        self.db = MySQLdb.connect(Configuration.configs['mysql_host'],
                                  Configuration.configs['mysql_usr'],
                                  Configuration.configs['mysql_pwd'],
                                  Configuration.configs['mysql_db'],
                                  charset="utf8")
        self.cursor = self.db.cursor()

    def close(self):
        self.cursor.close()
        self.db.close()

logging.basicConfig(filename=Configuration.configs['LogPath'],
                   level=logging.INFO,
                   format='%(asctime)s %(levelname)s %(message)s',
                   datefmt='%Y %b %d %H:%M:%S')
Logger = logging.getLogger()
