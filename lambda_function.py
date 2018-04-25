from __future__ import print_function
from oauth2client import service_account
from apiclient import discovery
from oauth2client import client
from oauth2client import tools
from oauth2client.file import Storage
from time import localtime

import httplib2
import os
import boto3
import json
import logging
import requests
import time
from string import find

def lambda_handler(event, context):
  logger = logging.getLogger()
  logger.setLevel(logging.INFO)
  sentFlag = 0
  
  lambdaClient = boto3.client('lambda')
  
  try: 
    event['batteryVoltage']
    currentTrigger = 'buttonPush'
  except:
    currentTrigger = 'apiCall'
    
  lastTrigger = os.environ['lastTrigger']
  currentTime = time.time()
  lastTime = float(os.environ['lastTime'])
  print(currentTime)
  
  if currentTime > lastTime + 5 or currentTrigger == lastTrigger:
    print(os.environ['lastTime'])
    lambdaClient.update_function_configuration(
          FunctionName='iotbutton_G030MD043261AFP0_iot-button-email-ses-python',
          Environment={
              'Variables': {
                  'lastTrigger': currentTrigger,
                  'lastReqId': os.environ['lastReqId'],
                  'lastTime': str(currentTime)
              }
          }
      )
    return 1
  
  if os.environ['lastReqId'] == context.aws_request_id : #exit if duplicate request
    print('Retry, exiting')
    return 1
    
  print(currentTrigger)

  print(os.environ['lastTrigger'])
  print(os.environ['lastReqId'])

  SCOPES = ['https://www.googleapis.com/auth/calendar']
  SERVICE_ACCOUNT_FILE = 'aws-iot-f3edea2d0394.json'

  credentials = service_account.ServiceAccountCredentials.from_json_keyfile_name(
          SERVICE_ACCOUNT_FILE, scopes=SCOPES)
          
  http = httplib2.Http()
  http = credentials.authorize(http)
  service = discovery.build('calendar', 'v3', http=http, cache_discovery = False)

  event = {
    "summary": "aws iot test",
    "location": "WINLAB",
    "description": "aws iot test - pushed from button",
    "start": {
      "dateTime": "2018-02-02T09:30:00-07:00",
      "timeZone": "America/Los_Angeles"
    },
    "end": {
      "dateTime": "2018-02-02T17:00:00-07:00",
      "timeZone": "America/Los_Angeles"
    }
  }
  
  sentFlag = 1
  print('sending event')
  hrui = service.events().insert(calendarId='jon.scott9013@gmail.com', body=event).execute()
    
  #if currentTrigger == 'apiCall' and os.environ['lastTrigger'] == 'buttonPush' :
  #  sentFlag = 1
  #  print('sending event')
  #  hrui = service.events().insert(calendarId='jon.scott9013@gmail.com', body=event).execute()
    
  if sentFlag == 1 :
    lambdaClient.update_function_configuration(
          FunctionName='iotbutton_G030MD043261AFP0_iot-button-email-ses-python',
          Environment={
              'Variables': {
                  'lastTrigger': '0',
                  'lastReqId': '0',
                  'lastTime': str(currentTime)
              }
          }
      )
    sentFlag = 0
  else :
    lambdaClient.update_function_configuration(
            FunctionName='iotbutton_G030MD043261AFP0_iot-button-email-ses-python',
            Environment={
                'Variables': {
                    'lastTrigger': currentTrigger,
                    'lastReqId': context.aws_request_id,
                    'lastTime': str(currentTime)
                }
            }
        )
  
  return 1