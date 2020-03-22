import os
import csv
from datetime import datetime as dt
from datetime import timedelta

DATA_LOGS_FOLDER = "environment_sensordata_logs/"
DAY_HISTS_FOLDER = "environment_day_hists/"


for file_id, file in enumerate(os.listdir(DATA_LOGS_FOLDER)):
	if file.endswith(".csv"):

		sensor_ID = file.replace("sensor_", "").replace("_datalog.csv", "")

		with open(DATA_LOGS_FOLDER + 'sensor_{}_datalog.csv'.format(sensor_ID)) as csv_log_file:
			csv_reader = csv.reader(csv_log_file, delimiter=',')

			yesterday = (dt.now() - timedelta(1)).replace(hour=0, minute=0, second=0, microsecond=0)
			
			temperatures = []
			humidities = []
			pressures = []
			for idx, row in enumerate(csv_reader):
				
				if(idx > 0):
					date_of_entry = dt.strptime(row[0], '%d.%m.%Y')
					
					if date_of_entry == yesterday:
						temperatures.append(float(row[2]))
						humidities.append(float(row[3]))
						pressures.append(float(row[4]))
					
					elif date_of_entry > yesterday:
						break
			
			avg_temp = round(sum(temperatures) / len(temperatures), 2)
			min_temp = min(temperatures)
			max_temp =max(temperatures)

			avg_humi = round(sum(humidities) / len(humidities), 2)
			min_humi = min(humidities)
			max_humi =max(humidities)

			avg_pres = round(sum(pressures) / len(pressures), 2)
			min_pres = min(pressures)
			max_pres =max(pressures)

			data_entry = [yesterday.strftime('%d.%m.%Y'), avg_temp, min_temp, max_temp, avg_pres, min_pres, max_pres, avg_humi, min_humi, max_humi]

			with open(DAY_HISTS_FOLDER + 'sensor_{}_hist.csv'.format(sensor_ID), 'a') as csv_hist_file:
				csv_writer = csv.writer(csv_hist_file)

				csv_writer.writerow(data_entry)


