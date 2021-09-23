from math import ceil

import pyrebase
import tkinter as tk
import os

import csv
import matplotlib.pyplot as plt
import numpy as np
from datetime import datetime

from pathlib import Path

# configure firebase and GUI
from matplotlib.patches import Patch

config = {
    "apiKey": "your api key",
    "authDomain": "your auth domain",
    "databaseURL": "None",
    "projectId": "your project id",
    "storageBucket": "your storage bucket",
    "messagingSenderId": "your messagingSenderId": 
    "appId": "your appId",
    "serviceAccount": "your_serviceAccount.json"
}

bar_colors1 = ['#edca82', '#097770', '#e0cdbe', '#a9c0a6', '#ddc3a5', '#e0a96d']
bar_colors_pdr = ['#7a68a0', '#77c593', '#316879', '#f7bdb0', '#7fd7cc', '#989796']
bar_colors_ber = ['#1d3c45', '#d2601a', '#fff1e1', '#aed6dc', '#ff9a8d', '#4a536b']
colors_RSSI_scenario = ['#505160', '#68829e', '#aebd38', '#598234', '#003b46', '#07575b', '#66a5ad']
cr_outer_colors = ['#fff1e1', '#aed6dc', '#ff9a8d', '#4a536b']
cr_inner_colors = ['#1d3c45', '#d2601a']

leg_labels = ['SF 8, BW 125 kHz', 'SF 10, BW 125 kHz', 'SF 12, BW 125 kHz',
                          'SF 8, BW 250 kHz', 'SF 10, BW 250 kHz', 'SF 12, BW 250 kHz']

HEIGHT = 820
WIDTH = 1300

frame_color = "#ecf241"

root = tk.Tk()

canvas = tk.Canvas(root, height=HEIGHT, width=WIDTH)
canvas.pack()

frame = tk.Frame(root, bg=frame_color, bd=5)
frame.place(relx=0.5, rely=0.02, relwidth=0.85, relheight=0.17, anchor="n")
label = tk.Label(frame, text="Choose experiment", font=("Arial", 20))
label.place(relwidth=1, relheight=0.4)

exp_param_names = ["Experiment Number", "Transmissions per Iteration", "Time between transmissions",    # 0,1,2
                   "Transmission length",   # 3
                   "Random seed for payload generation", "auto-incrementing transmission content", "Description", # 4, 5, 6
                   "Target distance",   # 7
                   "Height of sender", "Height of receiver", "Environment", "state", "Frequencies", "Bandwidths",   # 8, 9, 10, 11, 12, 13
                   "Spreading factors", # 14
                   "Coding rates", "Power settings", "Iterations", "Duration", "Start time", "Sender latitude",     # 15, 16, 17, 18, 19, 20
                   "Sender longitude", "Sender altitude",   # 21, 22
                   "Receiver latitude", "Receiver longitude", "Receiver altitude", "Actual distance",   # 23, 24, 25, 26
                   "Altitude difference",   # 27
                   "Temperature", "Humidity", "Pressure", "Clouds", "Sender orientation", # 28, 29, 30, 31, 32
                   "Receiver orientation"   # 33
                   ]

### table configuration
param_selection = [19, 28, 29, 30, 31]
column_labels = []
row_categories = ['Experiment', 'Distance', 'Scenario']
units = ["", "", "s", "", "", "", "", "m", "m", "m", "", "", "", "", "", "", "", "", "s", "", "", "", "m", "", "", "m", "m", "m", "°C", "%", "hPa", "", "", ""]

for i, param in enumerate(exp_param_names):
    if i in param_selection:
        column_labels.append(param)

modes = ['RSSI/SNR', 'PDR', 'BER', 'RSSI per scenario', 'CR', 'ETX/time-on-air/duty cycle', 'info tables']

import firebase_admin
from firebase_admin import credentials

cred = credentials.Certificate("your_serviceAccount.json")
firebase_admin.initialize_app(cred)

firebase = pyrebase.initialize_app(config)
storage = firebase.storage()

# get all experiment data from firebase if not already downloaded
files = storage.list_files()
for file in files:
    exp_num = file.name.split('/')[0][10:]

    p_1 = Path('experiment_data')
    if not p_1.exists():
        print("making experiment_data dir")
        os.mkdir("experiment_data")

    p_2 = Path('experiment_data/experiment' + exp_num)

    if not p_2.exists():
        os.mkdir("experiment_data/experiment" + exp_num)
        path_info = "experiment" + exp_num + "/experiment_info" + exp_num + ".txt"
        storage.child(path_info).download("experiment_data/experiment" + exp_num + "/experiment_info.txt")
        path_data = "experiment" + exp_num + "/experiment_data" + exp_num + ".csv"
        storage.child(path_data).download("experiment_data/experiment" + exp_num + "/experiment_data.csv")

OPTIONS = []

import glob

for file in (glob.glob("experiment_data/*")):
    with open(file + "/experiment_info.txt", 'r') as txt:
        lines = txt.readlines()
        OPTIONS.append(lines[6][:-1] + ", " + lines[10][:-1] + " : " + file.split('\\')[
            1])  # lines[6] contains description, lines[10] environment

OPTIONS = (sorted(OPTIONS))
drop_down_content = tk.StringVar(frame)
drop_down_content.set(OPTIONS[0])  # default value

mode = modes[0]
exp_counter = 0  # used if visualisation takes data from more than one experiment
label_exp_counter = tk.Label(frame, text='--', font=("Arial", 22))
label_exp_counter.place(relwidth=0.14, relheight=0.4, relx=0.86, rely=0.55)

# data arrays and other variables for different plots
pdr_arr = [[], [], [], [], [], []]
pdr_corr_arr = [[], [], [], [], [], []]
pdr_uncorr_arr = [[], [], [], [], [], []]

ber_arr = [[], [], [], [], [], []]
rssi_arr = []
rssi_arr_distances = []
snr_averaging_arr = []
rssi_arr_snr = []

etx_arr = [[], [], [], [], [], []]

info_arrays = []

# process and visualize data for selected experiment
def process_data(selection):
    experiment_number = str(selection).split('t')[-1]

    #########################################
    ########### get data ####################
    p_1 = Path('experiment_data')
    if not p_1.exists():
        os.mkdir("experiment_data")

    p_2 = Path('experiment_data/experiment' + experiment_number)

    if not p_2.exists():
        os.mkdir("experiment_data/experiment" + experiment_number)
        path_info = "experiment" + experiment_number + "/experiment_info" + experiment_number + ".txt"
        storage.child(path_info).download("experiment_data/experiment" + experiment_number + "/experiment_info.txt")
        path_data = "experiment" + experiment_number + "/experiment_data" + experiment_number + ".csv"
        storage.child(path_data).download("experiment_data/experiment" + experiment_number + "/experiment_data.csv")

    overview_right = ""
    overview_middle = ""
    overview_left = ""

    exp_params = {}
    exp_params_arr = []

    ########################################################
    ########### visualise experiment information ###########
    i = 0
    with open('experiment_data/experiment' + experiment_number + '/experiment_info.txt', 'r') as file:
        for line in file:
            exp_params[i] = line.strip('\n')
            if i in param_selection:
                if i != 19 and i != 28:
                    exp_params_arr.append(line.strip('\n') + units[i])
                else:
                    if i == 19:
                        ts = int(line.strip('\n'))
                        exp_params_arr.append(datetime.utcfromtimestamp(ts).strftime('%Y-%m-%d %H:%M:%S')[11:-3])
                    else:
                        exp_params_arr.append(str(round((float(line.strip('\n')) - 273.15), 1)) + units[i])
            if i < 11:
                overview_left += exp_param_names[i]
                overview_left += ": " + line + "\n\n"
            elif i < 22:
                overview_middle += exp_param_names[i]
                overview_middle += ": " + line + "\n\n"
            else:
                overview_right += exp_param_names[i]
                overview_right += ": " + line + "\n\n"
            i += 1

    frame_overview_left = tk.Frame(root, bg=frame_color, bd=5)
    frame_overview_left.place(relx=0.15, rely=0.21, relwidth=0.25, relheight=0.65, anchor="n")
    label_descr_integration = tk.Label(frame_overview_left, anchor="n", justify=tk.LEFT, text=overview_left)
    label_descr_integration.place(relwidth=1, relheight=1)

    frame_overview_middle = tk.Frame(root, bg=frame_color, bd=5)
    frame_overview_middle.place(relx=0.5, rely=0.21, relwidth=0.25, relheight=0.65, anchor="n")
    label_descr_integration = tk.Label(frame_overview_middle, anchor="n", justify=tk.LEFT, text=overview_middle)
    label_descr_integration.place(relwidth=1, relheight=1)

    frame_overview_right = tk.Frame(root, bg=frame_color, bd=5)
    frame_overview_right.place(relx=0.85, rely=0.21, relwidth=0.25, relheight=0.65, anchor="n")
    label_descr_integration = tk.Label(frame_overview_right, anchor="n", justify=tk.LEFT, text=overview_right)
    label_descr_integration.place(relwidth=1, relheight=1)

    ################################################
    ########### extract esperiment data ############
    total_number_vals = []
    identifier_vals = []
    rssi_vals = []
    snr_vals = []
    bit_error_vals = []

    last_row_total_number = 0;
    with open("experiment_data/experiment" + experiment_number + "/experiment_data.csv", "r") as csv_file:
        for row in csv.DictReader((x.replace('\0', '') for x in csv_file), delimiter=','):
            total_number_val = row['totalnumber']
            if total_number_val == last_row_total_number:
                continue
            total_number_vals.append(total_number_val)
            last_row_total_number = total_number_val
            identifier_vals.append(row['identifier'])
            rssi_vals.append(int(row['rssi']))
            snr_vals.append(int(row['snr']))
            bit_error_vals.append(int(row['biterrors']))

    #######################################################
    ############ aggregate results ########################
    fr_code = int(exp_params[12])
    bw_code = int(exp_params[13])
    sf_code = int(exp_params[14])
    cr_code = int(exp_params[15])
    pw_code = int(exp_params[16])

    number_of_transmissions = int(exp_params[1])
    packet_length = int(exp_params[3])
    print(packet_length)

    label = ["", "", "", "", ""]
    frequency_labels = ["869.5 MHz", "868.1 MHz", "868.3 MHz", "868.5 MHz", "867.1 MHz", "867.3 MHz", "867.5 MHz",
                        "867.7 MHz"]
    bandwidth_labels = ["125kHz", "250kHz", "500kHz"]
    coding_rate_labels = ["4/5", "4/6", "4/7", "4/8"]

    iteration = 0
    index = 0

    rounds = []
    rssi_iteration = []
    snr_iteration = []
    colors = []
    global exp_counter

    global rssi_arr
    global rssi_arr_distances
    global rssi_arr_snr
    global snr_averaging_arr
    distance = int(float(exp_params[26]))
    cr_vals = []
    cr_corrupted = []

    if not rssi_arr_distances or rssi_arr_distances[-1] != distance:
        rssi_arr.append([[], [], [], [], [], []])
        rssi_arr_distances.append(int(float(exp_params[26])))
        print(int(float(exp_params[26])))

        if snr_averaging_arr:
            rssi_arr_snr.append(np.mean(np.array(snr_averaging_arr)))  # append the avg snr for one scenario distance
            snr_averaging_arr = []

    for i in range(8):
        if ((fr_code >> i) & 1) == 1:
            label[0] = "Frequency: " + frequency_labels[i]
        else:
            continue
        for j in range(3):
            if ((bw_code >> j) & 1) == 1:
                label[1] = "Bandwidth: " + bandwidth_labels[j]
            else:
                continue
            for k in range(6):
                if ((sf_code >> k) & 1) == 1:
                    label[2] = "Spreading Factor: " + str(k + 7)
                else:
                    continue
                for l in range(4):
                    if ((cr_code >> l) & 1) == 1:
                        label[3] = "Coding Rate: " + coding_rate_labels[l]
                    else:
                        continue
                    for m in range(8):
                        if ((pw_code >> m) & 1) == 1:
                            label[4] = "Power: " + str(18 - m * 2)

                            rssi_sum = 0
                            biterror_sum = 0
                            corrupted_msgs_count = 0
                            for n in range(number_of_transmissions):
                                if index > len(rssi_vals) - 1:
                                    break

                                if n == int(total_number_vals[index]) - (
                                        iteration * number_of_transmissions):  # once the loop is at the right x value
                                    rounds.append(n)
                                    rssi_val = rssi_vals[index]
                                    rssi_iteration.append(rssi_val)
                                    snr_iteration.append(rssi_val - snr_vals[index])
                                    rssi_sum += rssi_val
                                    biterror_sum += bit_error_vals[index]
                                    if 'f' not in identifier_vals[index]:
                                        color = bar_colors1[1]
                                    else:
                                        color = '#f07c3a'
                                        corrupted_msgs_count += 1
                                    index += 1
                                    colors.append(color)
                        else:
                            continue
                        iteration += 1
                        if (len(rssi_iteration) > 0):
                            rssi_avg = rssi_sum / len(rssi_iteration)
                            if corrupted_msgs_count > 0:
                                bit_err_rate = biterror_sum / (corrupted_msgs_count * packet_length * 8)
                            else:
                                bit_err_rate = 0
                            pdr = len(rssi_iteration) / number_of_transmissions
                            pdr_corr = corrupted_msgs_count / number_of_transmissions
                            uncorr_count = (len(rssi_iteration) - corrupted_msgs_count)
                            pdr_uncorr =  uncorr_count / number_of_transmissions
                            etx = number_of_transmissions / (len(rssi_iteration) - corrupted_msgs_count) if uncorr_count > 0 else 0
                        else:
                            rssi_avg = 0
                            bit_err_rate = 0
                            pdr = 0
                            etx = 0
                            pdr_corr = 0
                            pdr_uncorr = 0

                        ####################################################################################################
                        ########## mode to illustrate every iterations received messages as RSSI values with noise floor####
                        if mode == modes[0]:
                            plt.scatter(rounds, rssi_iteration, color=colors)
                            plt.plot(rounds, snr_iteration, color=bar_colors1[3], label=('noise floor'));
                            plt.scatter([], [], color=bar_colors1[1], label='rssi value of correct messages')
                            plt.scatter([], [], color='#f07c3a', label='rssi value of corrupted messages')

                            plt.xlabel('message number', fontsize=12)
                            plt.ylabel('dBm', fontsize=12)
                            label_str = ""
                            for p in label:
                                label_str += p + ", "
                            label_str += '\naverage RSSI: ' + str(
                                round(rssi_avg, 2)) + ',\npacket delivery rate: ' + str(round(
                                pdr, 2)) + '\naverage biterror rate: ' + str(
                                round(bit_err_rate, 4))
                            #plt.title(label_str, fontsize=6)

                            plt.legend(framealpha=0.4, prop={'size': 7})
                            plt.savefig('plots/below_noise_floor_corr.png', bbox_inches="tight", dpi=300)
                            plt.show()

                        #######################################################################################
                        ############ mode to illustrate PDR with error bar over several experiments ###########
                        ########### (preparation while looping through single experiments' iterations #########
                        if mode == modes[1]:
                            global pdr_arr
                            global pdr_corr_arr
                            global pdr_uncorr_arr
                            pdr_arr[iteration - 1].append(pdr)
                            pdr_corr_arr[iteration - 1].append(pdr_corr)
                            pdr_uncorr_arr[iteration - 1].append(pdr_uncorr)

                        ##########################################################################################
                        ############ mode to illustrate bit error rate with error bar over several experiments ###
                        ########### (preparation while looping through single experiments' iterations ############
                        if mode == modes[2]:
                            global ber_arr
                            ber_arr[iteration - 1].append(bit_err_rate)

                        ##########################################################################################
                        ############ mode to illustrate rssi for several ranges with several LoRa parameter ######
                        ############ combinations with error bar over several experiments for a whole scenario ###
                        ########### (preparation while looping through single experiments' iterations ############
                        if mode == modes[3]:
                            if rssi_avg != 0:
                                rssi_arr[len(rssi_arr_distances) - 1][iteration - 1].append(rssi_avg)
                                snr_averaging_arr += snr_iteration

                        ##########################################################################################
                        ############ mode to illustrate difference in coding rates for one LoRa parameter  #######
                        ############ combination as pie chart ####################################################
                        if mode == modes[4]:
                            if len(rssi_iteration) != 0:
                                cr_vals.append([len(rssi_iteration) - corrupted_msgs_count, corrupted_msgs_count])
                                cr_corrupted.append('CR 4/' + str(((iteration - 1) % 4) + 5) + "\n" + str(int((corrupted_msgs_count / len(rssi_iteration)) * 100)) + "% corrupted")

                            if iteration % 4 == 0:
                                fig2, ax2 = plt.subplots()
                                size = 0.3
                                vals = np.array(cr_vals)

                                if iteration == 16:
                                    ax2.pie(vals.sum(axis=1), radius=1, colors=['#aed6dc'], labels=cr_corrupted,
                                           wedgeprops=dict(width=size, edgecolor='w'))
                                else:
                                    ax2.pie(vals.sum(axis=1), radius=1, colors=cr_outer_colors, labels=cr_corrupted,
                                            wedgeprops=dict(width=size, edgecolor='w'))

                                ax2.pie(vals.flatten(), radius=1 - size, colors=cr_inner_colors,
                                       wedgeprops=dict(width=size, edgecolor='w'))

                                ax2.set(aspect="equal")

                                legend_elements = [Patch(facecolor=cr_inner_colors[0], label='correct messages'),
                                                   Patch(facecolor=cr_inner_colors[1], label='corrupted messages')]

                                plt.legend(handles=legend_elements, bbox_to_anchor=(1.2, 1.065), loc='upper right')
                                plt.savefig('plots/test_CR' + str(iteration) + '.png', bbox_inches="tight", dpi=300)
                                plt.show()
                                cr_vals = []
                                cr_corrupted = []

                        #######################################################################################
                        ############ mode to illustrate ETX  with error bar over several experiments ##########
                        ########### (preparation while looping through single experiments' iterations #########
                        if mode == modes[5]:
                            global etx_arr
                            etx_arr[iteration - 1].append(etx)

                        rounds = []
                        rssi_iteration = []
                        snr_iteration = []
                        colors = []

    print(mode)
    ##############################################################
    ######### plot for PDR mode ##################################
    if mode == modes[1]:
        exp_counter += 1
        label_exp_counter.config(text='choose\n' + str(exp_counter + 1) + '. experiment', font=("Arial", 12))

        if exp_counter == 3:
            exp_counter = 0
            label_exp_counter.config(text=str(exp_counter))

            string_labels = ['8/1', '10/1', '12/1', '8/2', '10/2', '12/2']
            leg_labels = ['SF 8, BW 125 kHz', 'SF 10, BW 125 kHz', 'SF 12, BW 125 kHz',
                          'SF 8, BW 250 kHz', 'SF 10, BW 250 kHz', 'SF 12, BW 250 kHz']

            means_pdr = []
            stds_pdr = []
            means_pdr_corr = []
            stds_pdr_corr = []
            means_pdr_uncorr = []
            stds_pdr_uncorr = []

            for arr in pdr_arr:
                np_arr = np.array(arr)
                means_pdr.append(np.mean(np_arr))
                stds_pdr.append(np.std(np_arr))
            for arr in pdr_corr_arr:
                np_arr = np.array(arr)
                means_pdr_corr.append(np.mean(np_arr))
                stds_pdr_corr.append(np.std(np_arr))
            for arr in pdr_uncorr_arr:
                np_arr = np.array(arr)
                means_pdr_uncorr.append(np.mean(np_arr))
                stds_pdr_uncorr.append(np.std(np_arr))

            width = 0.8

            ######### plot pdr ##########
            fig1, ax1 = plt.subplots(figsize=(5, 7))
            ax1.bar(string_labels, means_pdr, width, yerr=stds_pdr, color=bar_colors_pdr, capsize=4,
                    label=leg_labels)

            ax1.set_ylim(0, 1.2)
            ax1.set_ylabel('PDR', fontsize=20)

            ax1.set_xticks(np.arange(len(pdr_arr)))
            ax1.tick_params(axis='both', labelsize=16)
            ax1.tick_params(direction='in', bottom=True, left=True, right=True, top=True, width=2)

            plt.setp(ax1.spines.values(), linewidth=2)
            plt.tight_layout()

            legend_elements = []
            for e in range(6):
                legend_elements.append(Patch(facecolor=bar_colors_pdr[e], label=leg_labels[e]))

            plt.legend(handles=legend_elements, loc='upper right', framealpha=0.4,
                       prop={'size': 7})
            plt.savefig('plots/sea_pdr1.png', bbox_inches="tight", dpi=400)
            plt.show()

            ########## plot with distinguishing corrupted/uncorrupted messages #########
            fig2, ax2 = plt.subplots(figsize=(5, 7))
            ax2.bar(string_labels, means_pdr_uncorr, width, yerr=stds_pdr_uncorr, color=bar_colors_pdr, capsize=4, label='uncorrupted')
            ax2.bar(string_labels, means_pdr_corr, width, error_kw=dict(ecolor='#a89894', lw=2, capsize=4, capthick=1),
                    yerr=stds_pdr_corr, bottom=means_pdr_uncorr, color='#fc6265', label='corrupted')

            ax2.set_ylim(0, 1.1)
            ax2.set_ylabel('PDR', fontsize=20)

            ax2.set_xticks(np.arange(len(pdr_arr)))
            ax2.tick_params(axis='both', labelsize=16)
            ax2.tick_params(direction='in', bottom=True, left=True, right=True, top=True, width=2)

            plt.setp(ax2.spines.values(), linewidth=2)
            plt.tight_layout()

            legend_elements.append(Patch(facecolor='#fc6265', label='corrupted part'))
            plt.legend(handles=legend_elements, loc='lower right', framealpha=0.4,
                       prop={'size': 7})
            plt.savefig('plots/2700_cityL_pdr2.png', bbox_inches="tight", dpi=400)
            plt.show()

            pdr_arr = [[], [], [], [], [], []]
            pdr_corr_arr = [[], [], [], [], [], []]
            pdr_uncorr_arr = [[], [], [], [], [], []]

    ##############################################################
    ######### plot for BER mode ##################################
    if mode == modes[2]:
        exp_counter += 1
        label_exp_counter.config(text='choose\n' + str(exp_counter + 1) + '. experiment', font=("Arial", 12))

        if exp_counter == 3:
            exp_counter = 0
            label_exp_counter.config(text=str(exp_counter))

            leg_labels = ['SF 8, BW 125 kHz', 'SF 10, BW 125 kHz', 'SF 12, BW 125 kHz',
                          'SF 8, BW 250 kHz', 'SF 10, BW 250 kHz', 'SF 12, BW 250 kHz']

            fig2, ax2 = plt.subplots(figsize=(5, 7))
            location = 0
            for arr in ber_arr:
                np_arr = np.array(arr)
                ax2.bar(location, np.mean(np_arr), yerr=np.std(np_arr), align='center', ecolor='black',
                       label=leg_labels[location], capsize=7, color=bar_colors_ber[location])
                location += 1

            ax2.set_yscale('linear')
            ax2.set_ylim(0, 0.25)
            ax2.set_ylabel('BER', fontsize=20)

            ax2.set_xticks(np.arange(len(ber_arr)))
            empty_string_labels = ['8/1', '10/1', '12/1', '8/2', '10/2', '12/2']
            ax2.set_xticklabels(empty_string_labels)

            ax2.tick_params(axis='both', labelsize=16)
            ax2.tick_params(direction='in', bottom=True, left=True, right=True, top=True, width=2)

            plt.setp(ax2.spines.values(), linewidth=2)
            plt.tight_layout()

            plt.legend(loc='upper right', framealpha=0.4, prop={'size': 7})
            plt.savefig('plots/ber_500_forest.png', bbox_inches="tight", dpi=400)
            plt.show()

            ber_arr = [[], [], [], [], [], []]

    ##############################################################
    ######### plot for RSSI/range mode ##################################
    if mode == modes[3]:
        exp_counter += 1
        label_exp_counter.config(text='choose\n' + str(exp_counter + 1) + '. experiment', font=("Arial", 12))

        if exp_counter == 12:
            exp_counter = 0
            label_exp_counter.config(text=str(exp_counter))

            rssi_arr_snr.append(np.mean(np.array(snr_averaging_arr)))  # append the avg snr for one scenario distance
            snr_averaging_arr = []
            fig2 = plt.figure()
            x_arr = np.array([200, 400, 600, 800])
            # x_arr = np.array(rssi_arr_distances) (instead of above line for other representation)

            leg_labels = ['SF 8, BW 125 kHz', 'SF 10, BW 125 kHz', 'SF 12, BW 125 kHz',
                          'SF 8, BW 250 kHz', 'SF 10, BW 250 kHz', 'SF 12, BW 250 kHz']

            zero_size_dev_arrays = []
            for graph in range(6):
                y_arr = []
                y_err_arr = []
                for distance in range(len(rssi_arr)):
                    dev_arr = np.array(rssi_arr[distance][graph])
                    if dev_arr.size != 0:
                        y_arr.append(np.mean(dev_arr))
                        y_err_arr.append(np.std(dev_arr))
                    else:
                        zero_size_dev_arrays.append(distance)

                y = np.array(y_arr)
                yerr_graph = np.array(y_err_arr)

                x_arr_sub = np.delete(x_arr, zero_size_dev_arrays)
                zero_size_dev_arrays = []
                lines = {'linestyle': 'None'}       # remove for other representation
                plt.rc('lines', **lines)            # remove for other representation
                plt.errorbar(x_arr_sub + 9*graph, y, yerr=yerr_graph, elinewidth=5, color=colors_RSSI_scenario[graph], label=leg_labels[graph])
                # plt.errorbar(x_arr_sub, y, yerr=yerr_graph, color=colors_RSSI_scenario[graph], label=leg_labels[graph]) (instead of above line for other representation)

            plt.plot(x_arr + 18, rssi_arr_snr, color=colors_RSSI_scenario[6], label='noise floor', linestyle='--',
                     marker='o');
            plt.xlabel('distance', fontsize=12)
            plt.ylabel('dBm', fontsize=12)
            plt.legend(loc='lower right', framealpha=0.4, prop={'size': 7})
            plt.savefig('plots/test_RSSI_per_scenario_nonelev.png', bbox_inches="tight", dpi=300)
            plt.show()

            rssi_arr = []
            rssi_arr_distances = []
            rssi_arr_snr = []

    ##############################################################
    ######### plot for ETX mode ##################################
    if mode == modes[5]:
        exp_counter += 1
        label_exp_counter.config(text='choose\n' + str(exp_counter + 1) + '. experiment', font=("Arial", 12))

        if exp_counter == 3:
            exp_counter = 0
            label_exp_counter.config(text=str(exp_counter))

            leg_labels = ['SF 8, BW 125 kHz', 'SF 10, BW 125 kHz', 'SF 12, BW 125 kHz',
                          'SF 8, BW 250 kHz', 'SF 10, BW 250 kHz', 'SF 12, BW 250 kHz']

            fig2, ax2 = plt.subplots(figsize=(5, 7))
            location = 0
            sf_s = [8, 10, 12, 8, 10, 12]
            bw_s = [125, 125, 125, 250, 250, 250]
            for i, arr in enumerate(etx_arr):
                time_on_air = 8 * packet_length - 4 * sf_s[i] + 28 + 16
                ldr_opt = (sf_s[i] == 11 and bw_s[i] == 125) or (sf_s[i] == 12)
                time_on_air /= (4 * sf_s[i] - 2) if ldr_opt else (4 * sf_s[i])
                time_on_air = ceil(time_on_air) if (time_on_air > 0) else 0
                time_on_air = time_on_air * (2 + 4) + 8;

                symbol_duration = (1 << sf_s[i]) / bw_s[i]
                time_on_air *= symbol_duration
                time_on_air += 12.25 * symbol_duration
                print("time-on-air: " + str(time_on_air))

                comb_arr = []
                for x in arr:
                    # time for one transmission (first transmission is duty cycle penalty free) +
                    # additional time for waiting in compliance with duty cycle limitations (for 869.5 MHz)
                    if x != 0:
                        comb_arr.append(time_on_air + ((x - 1) * time_on_air * 10))


                np_arr = np.array(comb_arr)
                mean = 0
                std = 0
                if np_arr.size != 0:
                    mean = np.mean(np_arr)
                    std = np.std(np_arr)
                    if np_arr.size < 3:
                        std = 100000        # 'infinity' error bar if some subexperiments received nothing (for them it would take infinitely long to receive a message), but others did
                if mean == 0:
                    ax2.bar(location, mean, align='center', ecolor='black',
                        label=leg_labels[location], capsize=7, color=colors_RSSI_scenario[location])
                else:
                    ax2.bar(location, mean, yerr=std, align='center', ecolor='black',
                        label=leg_labels[location], capsize=7, color=colors_RSSI_scenario[location])

                if np_arr.size < 3 and np_arr.size != 0:        # change color for 'infinite' error bar to distinguish from other error bars going over whole screen
                    ax2.bar(location, mean,error_kw = dict(ecolor='#a89894', lw=2, capsize=4, capthick=1), yerr=std, align='center', ecolor='black',
                             capsize=7, color=colors_RSSI_scenario[location])

                location += 1

            ax2.set_yscale('linear')
            ax2.set_ylim(0, 2000)
            ax2.set_ylabel('Time for successful transmission in ms', fontsize=20)

            ax2.set_xticks(np.arange(len(ber_arr)))
            empty_string_labels = ['8/1', '10/1', '12/1', '8/2', '10/2', '12/2']
            ax2.set_xticklabels(empty_string_labels)

            ax2.tick_params(axis='both', labelsize=16)
            ax2.tick_params(direction='in', bottom=True, left=True, right=True, top=True, width=2)

            plt.setp(ax2.spines.values(), linewidth=2)
            plt.tight_layout()

            plt.legend(loc='upper right', numpoints=1, framealpha=0.4, prop={'size': 7})
            plt.savefig('plots/30_cityL_etx2.png', bbox_inches="tight", dpi=300)
            plt.show()

            etx_arr = [[], [], [], [], [], []]

    ##############################################################
    ######### plot tables ##################################
    if mode == modes[6]:
        row_category = 0
        exp_counter += 1
        label_exp_counter.config(text='choose\n' + str(exp_counter + 1) + '. ' + row_categories[row_category], font=("Arial", 12))
        info_arrays.append(exp_params_arr)

        if exp_counter == 4:
            exp_counter = 0
            label_exp_counter.config(text=str(exp_counter))

            row_labels = []
            for i, arr in enumerate(info_arrays):
                row_labels.append(row_categories[row_category] + ' ' + str(i + 1))

            row_colors = plt.cm.BuPu(np.full(len(row_labels), 0.1))
            col_colors = plt.cm.BuPu(np.full(len(column_labels), 0.1))

            plt.figure(linewidth=2, figsize=(15,2))

            table = plt.table(cellText=info_arrays, rowLabels=row_labels, rowColours=row_colors,
                                  rowLoc='right', colColours=col_colors, colLabels=column_labels,
                                  loc='center')
            table.scale(1, 1.5)

            ax = plt.gca()
            ax.get_xaxis().set_visible(False)
            ax.get_yaxis().set_visible(False)

            plt.box(on=None)

            plt.savefig('plots/table_test.png', bbox_inches="tight", dpi=300)
            plt.show()



# triggered when choosing a mode from drop down menu
def set_mode(selection):
    global mode
    mode = selection

    if mode != modes[0] and mode != modes[4]:
        label_exp_counter.config(text='choose\n' + str(exp_counter + 1) + '. experiment', font=("Arial", 12))
    else:
        label_exp_counter.config(text='--', font=("Arial", 22))

    if mode == modes[6]:
        global info_arrays
        info_arrays = []


# GUI dropdown experiments
from tkinter import font as tkFont

# experiment drop down menu
drop_down = tk.OptionMenu(frame, drop_down_content, *OPTIONS, command=process_data)
helv20 = tkFont.Font(family='Helvetica', size=14)
drop_down.config(font=helv20)
drop_down.place(rely=0.5, relx=0.01, relwidth=0.57, relheight=0.5)

OPTIONS2 = []

# mode drop down menu
for x in range(0, len(modes)):
    OPTIONS2.append(modes[x])

drop_down_content2 = tk.StringVar(frame)
drop_down_content2.set(OPTIONS2[0])  # default value

drop_down_mode = tk.OptionMenu(frame, drop_down_content2, *OPTIONS2, command=set_mode)
helv20 = tkFont.Font(family='Helvetica', size=14)
drop_down_mode.config(font=helv20)
drop_down_mode.place(rely=0.5, relx=0.59, relwidth=0.25, relheight=0.5)

root.mainloop()
