# coding=utf-8
from util import Configuration,Logger
import os


if __name__ == '__main__':
    if Configuration.configs['PythonVersion'] == '3':
        python = "python3"
    else:
        python = "python"

    Logger.info("===================Begin===================")

    # Step1. Initialize the database
    Logger.info("Step1. Initialize the database.")
    sql_str = "mysql -u%s -p%s -e 'drop database if exists %s; create database %s; use %s; source '%s';'" \
              % (Configuration.configs['mysql_usr'],
                 Configuration.configs['mysql_pwd'],
                 Configuration.configs['mysql_db'],
                 Configuration.configs['mysql_db'],
                 Configuration.configs['mysql_db'],
                 Configuration.configs['TraceSqlPath'])
    result = os.system(sql_str)
    if result != 0:
        Logger.error("Fail to create the database")
        exit()

    # Step2. Import trace data into the database
    Logger.info("Step2. Import trace data to the database.")
    result = os.system("%s %s/REP/recordToDatabase.py" % (python, Configuration.configs['ProjectPath']))
    if result != 0:
        Logger.error("Fail to import trace data to the database")
        exit()

    # # Step2-2. Import log data into the database
    # Logger.info("Step2-2. Import log data to the database.")
    # result = os.system("%s %s/REP/process_log.py" % (python, Configuration.configs['ProjectPath']))
    # if result != 0:
    #     Logger.error("Fail to import log data to the database in Step2-2.")
    #     exit()

    # Step3. Filter unused trace data
    Logger.info("Step3. Filter unused trace data.")
    sql_str = "mysql -u%s -p%s -e \
    'use %s; \
    drop table if exists no_filter;\n \
    create table no_filter(select * from trace_info where dtype<>4 and dtype<>10);\n \
    drop table if exists no_unix;\n \
    create table no_unix(select * from no_filter);\n \
    drop table if exists no_unix2; \n \
    create table no_unix2(select * from no_filter where (ftype <> 2 and ftype <> 4) or ((ftype = 2 or ftype = 4) and ((rlength = 0 or rlength = -1) or data_id <> \"\"))); \n \
    '" % (Configuration.configs['mysql_usr'],
          Configuration.configs['mysql_pwd'],
          Configuration.configs['mysql_db'])
    result = os.system(sql_str)
    if result != 0:
        Logger.error("Fail to filter unused trace data.")
        exit()

    # Step4. Aggregate all trace/log events
    Logger.info("Step4. Aggregate all trace/log events.")
    result = os.system("%s %s/REP/aggregate_events.py" % (python, Configuration.configs['ProjectPath']))
    if result != 0:
        Logger.error("Fail to aggregate all trace/log events in step 4.")
        exit()
    sql_str = "mysql -u%s -p%s -e \
    'use %s; \
    update all_trace_events set data_id = NULL where data_id not like \"@%%@\"; \
    update all_trace_events set data_unit_id = NULL where data_unit_id not like \"@%%@\"; \
    '" % (Configuration.configs['mysql_usr'],
          Configuration.configs['mysql_pwd'],
          Configuration.configs['mysql_db'])
    result = os.system(sql_str)
    if result != 0:
        Logger.error("Fail to update all_trace_events in step 4.")
        exit()
    result = os.system("%s %s/REP/aggregate_events.py reOrder" % (python, Configuration.configs['ProjectPath']))
    if result != 0:
        Logger.error("Fail to aggregate all trace/log events in step 4.")
        exit()

    # Step5. Stitch all trace/log events with traced causal relationship
    Logger.info("Step5. Stitch all trace/log events with traced causal relationship.")
    sql_str = "mysql -u%s -p%s -e \
    'use %s; \
    update combination1 set data_id = NULL where data_id not like \"@%%@\"; \
    update combination1 set data_unit_id = NULL where data_unit_id not like \"@%%@\"; \
    alter table combination1 add column l2_index int;\
    alter table combination1 add column l2_length int;\
    alter table combination1 add column l2_index_alt TINYINT default 0; \
    alter table combination1 add column l2_parent bigint(20); \
    alter table combination1 add column time_parent bigint(20); \
    '" % (Configuration.configs['mysql_usr'],
          Configuration.configs['mysql_pwd'],
          Configuration.configs['mysql_db'])
    result = os.system(sql_str)
    if result != 0:
        Logger.error("Fail to update all_trace_events in step 5.")
        exit()
    result = os.system("%s %s/REP/merge.py" % (python, Configuration.configs['ProjectPath']))
    if result != 0:
        Logger.error("Fail to identifying causal relationships among trace events in step 5.")
        exit()

    # Step6. Abstract the execution path to a higher level
    Logger.info("Step6. Abstract the execution path to a higher level.")
    result = os.system("%s %s/REP/abstract.py" % (python, Configuration.configs['ProjectPath']))
    if result != 0:
        Logger.error("Fail to abstract the execution path to a higher level in step 6.")
        exit()

    # sql_str = "mysql -u%s -p%s -e 'use %s;select count(*) from all_events;'" \
    #           % (Configuration.configs['mysql_usr'],
    #              Configuration.configs['mysql_pwd'],
    #              Configuration.configs['mysql_db'])
    # result = os.popen(sql_str).read().split("\n")[1]
    # Logger.info("Filtered trace events: %s" % result)
    Logger.info("===================END===================\n")
