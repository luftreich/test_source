import json
import os,sys 
import urllib
import urllib2
import httplib
import thread
import threading
import time
import logging
import re
import readline

dir_now=[]
speed_file_list=[]
json_file=0
version_list=['V1.0.808','V1.0.805','V1.0.1334','V1.0.1064','V1.0.1406','V1.0.1385']

def log_config():
    logfile='log'+'_parse_'+time.strftime("%Y-%m-%d_%H_%M_%S")
    logging.basicConfig(filename=logfile,filemode='w',level=logging.DEBUG);

def filter_speeds_txt(input_file):    	
    valid = re.compile(r"dcdn_speeds.\d{8}.txt")
    if valid.match(input_file) is not None:
        speed_file_list.append(input_file)
        return True
    else:
        return False

def parse_the_dir_list(dir_now):
	for tmp_file in dir_now:
		filter_speeds_txt(tmp_file)

def fun_init_json_result():
    global json_file
    json_file = open(os.getcwd()+'/parse_speed_result.json','w')
    #json_file.write('{'+'\n')

def fun_insert_into_json_result(json_item):
    global json_file
    json_file.write(json_item+','+'\n')

def fun_end_into_json_result():
    global json_file
    json_file.seek(-2,2)
    json_file.write('\n'+'}')
    json_file.close()
    print 'end json '

def parse_the_common_info(file_name):
    zero_speed_count=0
    valid_speed_count=0
    avgspeed_all=0
    minspeed_all=0
    maxspeed_all=0
    result={}
    for version in version_list:
        result[version]={}

    fd=open(os.getcwd()+'/data/'+file_name)
    fd.readline()
    for line in fd.readlines():
        match = re.match('(.*),(.*),(.*),(.*),(.*),(.*)',line)
        (sn,product_id,version,minspeed,maxspeed,avgspeed)=match.groups()
        if (int(minspeed),int(maxspeed),int(avgspeed)) == (0,0,0) :
            zero_speed_count = zero_speed_count+1 
        else:
            valid_speed_count = valid_speed_count+1
            avgspeed_all = avgspeed_all+int(avgspeed)
        if minspeed_all == 0:
            minspeed_all = int(minspeed)

        minspeed_all = min(minspeed_all,int(minspeed))
        maxspeed_all = max(maxspeed_all,int(maxspeed))
        

        if version in version_list:
            if (int(minspeed),int(maxspeed),int(avgspeed)) == (0,0,0) :
                    if result[version].has_key('no_speed') != True:
                        result[version]['no_speed']=0
                    else:
                        result[version]['no_speed']=result[version]['no_speed']+1
            else:
                if result[version].has_key('valid_speed') != True:
                    result[version]['valid_speed']=1
                else:
                    result[version]['valid_speed']=result[version]['valid_speed']+1
                
                if result[version].has_key('avg_speed') != True:
                    result[version]['avg_speed']=int(avgspeed)
                else:
                    result[version]['avg_speed']=result[version]['avg_speed']+int(avgspeed)
    
            result[version]['minspeed'] = minspeed_all;
            result[version]['maxspeed'] = maxspeed_all;
                
    fd.close()

    for version in version_list:
        if result[version].has_key('avg_speed') == True:
            result[version]['avg_speed']=result[version]['avg_speed']/result[version]['valid_speed']
    return (zero_speed_count,valid_speed_count,avgspeed_all/valid_speed_count,minspeed_all,maxspeed_all,result)


def parse_the_speed_file_list(file_list):
    global json_file
    json_string={}
    fun_init_json_result()
    for speed_file in file_list:
        #
        result={}
        data=re.findall(r'[0-9]{8}',speed_file) 
        (zero_speed_count,valid_speed_count,avgspeed,minspeed,maxspeed,version_result)=parse_the_common_info(speed_file)    
        result['no_speed']=zero_speed_count
        result['valid_speed']=valid_speed_count
        result['avg_speed']=avgspeed
        result['minspeed']=minspeed
        result['maxspeed']=maxspeed
        result['version_info']=version_result
        json_string[str(data[0])]=result
    item=json.dumps(json_string)
    #fun_insert_into_json_result(item)
    json_file.write(item)
    json_file.close()
    #fun_end_into_json_result()
       # break


if __name__ == '__main__':
    #log_confuuu
    try:
    	dir_now=os.listdir(os.getcwd()+'/data')
    except:
    	print 'invalid running dir,return'
    parse_the_dir_list(dir_now)
    parse_the_speed_file_list(speed_file_list)

