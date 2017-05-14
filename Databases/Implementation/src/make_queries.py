import psycopg2
from psycopg2.extensions import AsIs
import pandas as pd
import datetime
import sys
import os
import functools
import re

import connection

import my_logger
from my_logger import exception_handler
from my_logger import logger

import input_processing
from input_processing import read_and_process, read_while_not_set, read_while_not_valid

import db_queries

conn = None
cursor = None


@exception_handler(logger)
def get_help():
    print(
        "Records:\n"
        "|----/add_record: add new record to log\n"
        "|----/update_record: update existing record\n"
        "Patients:\n"
        "|----/create_patient_profile: add new patient's profile\n"
        "|----/update_patient_profile: update existing profile\n"
        "|----/patient_ids: display ids of patients with given name\n"
        "|----/patient_data: display personal data of patient with given id\n"
        "|----/patient_history: display all records related to patient with given id\n"
        "Doctors:\n"
        "|----/create_doctor_profile: add new doctor's profile\n"
        "|----/update_doctor_profile: update existing profile\n"
        "|----/doctor_ids: display ids of doctors with given name\n"
        "|----/doctor_data: display personal data of doctor with given id\n"
        "|----/set_appointment: schedule a visit to the doctor\n"
        "Queries and statistics:\n"
        "|----/show_table: show table with given name\n"
        "|----/get_sample: obtain all data satisfying given constraints\n"
        "Service:\n"
        "|----/help: show this help page\n"
        "|----/cancel: interrupt current command and choose another\n"
        "|----/exit: close database"
    )


@exception_handler(logger)
def add_record():
    try:
        db_queries.add_record()
    except Exception:
        print("Database error: can't add patient's record")
        raise


@exception_handler(logger)
def update_record():
    try:
        record_id = read_while_not_set("Record's id: ", on_bad_input="Id must be set")
    except Exception:
        raise
    if db_queries.check_id_existence(table_name='logs.medical_log', id_name='case_id', id_val=record_id):
        try:
            db_queries.update_record(record_id)
        except Exception:
            print("Database error: can't update patient's record")
            raise
    else:
        print("No data about such a patient's id")


@exception_handler(logger)
def create_patient_profile():
    try:
        db_queries.create_patient_profile()
    except Exception:
        print("Database error: can't create patient's profile")
        raise


@exception_handler(logger)
def update_patient_profile():
    try:
        patient_id = read_while_not_set("Patient's id: ", on_bad_input="Id must be set")
    except Exception:
        raise
    if db_queries.check_id_existence(table_name='logs.patients', id_name='patient_id', id_val=patient_id):
        try:
            db_queries.update_patient_profile(patient_id)
        except Exception:
            print("Database error: can't update patient's profile")
            raise
    else:
        print("No data about such a patient's id")


@exception_handler(logger)
def get_patient_ids():
    try:
        patient_name = read_while_not_set("Patient's name: ", on_bad_input="Patient's name can't be empty")
        db_queries.get_ids_by_pname(patient_name)
    except Exception:
        print("Can't get the ids")
        raise


@exception_handler(logger)
def get_patient_data():
    try:
        patient_id = read_while_not_set("Patient's id: ", on_bad_input="Id must be set")
    except Exception:
        raise
    if db_queries.check_id_existence(table_name='logs.patients', id_name='patient_id', id_val=patient_id):
        try:
            db_queries.show_patient_personal_data(patient_id)
        except Exception:
            print("Database error: can't show patient's data")
            raise
    else:
        print("No data about such a patient's id")


@exception_handler(logger)
def get_patient_history():
    try:
        patient_id = read_while_not_set("Patient's id: ", on_bad_input="Id must be set")
    except Exception:
        raise
    if db_queries.check_id_existence(table_name='logs.patients', id_name='patient_id', id_val=patient_id):
        try:
            db_queries.show_patient_history(patient_id=patient_id)
        except Exception:
            print("Database error: can't show patient's history")
            raise
    else:
        print("No data about such a patient's id")


@exception_handler(logger)
def create_doctor_profile():
    try:
        db_queries.create_doctor_profile()
    except Exception:
        print("Database error: can't create doctor's profile")
        raise


@exception_handler(logger)
def update_doctor_profile():
    try:
        doctor_id = read_while_not_set("Doctor's id: ", on_bad_input="Id must be set")
    except Exception:
        raise
    if db_queries.check_id_existence(table_name='logs.doctors', id_name='doctor_id', id_val=doctor_id):
        try:
            db_queries.update_doctor_profile(doctor_id)
        except Exception:
            print("Database error: can't update doctor's profile")
            raise
    else:
        print("No data about such a doctor's id")


@exception_handler(logger)
def get_doctor_ids():
    try:
        doctor_name = read_while_not_set("Doctor's name: ", on_bad_input="Doctor's name can't be empty")
        db_queries.get_ids_by_dname(doctor_name)
    except Exception:
        print("Can't get the ids")
        raise


@exception_handler(logger)
def get_doctor_data():
    try:
        doctor_id = read_and_process("Type the doctor's id: ")
    except Exception:
        raise
    if db_queries.check_id_existence(table_name='logs.doctors', id_name='doctor_id', id_val=doctor_id):
        try:
            db_queries.show_doctor_personal_data(doctor_id)
        except Exception:
            print("Database error: can't show doctor's data")
            raise
    else:
        print("No data about such a doctor's id")


@exception_handler(logger)
def set_appointment():
    try:
        patient_id = read_while_not_set("Patient's id: ", on_bad_input="Id must be set")
        doctor_id = read_while_not_set("Doctor's id: ", on_bad_input="Id must be set")
        db_queries.add_session(patient_id, doctor_id)
    except Exception:
        print("Database error: can't add a session")
        raise


@exception_handler(logger)
def show_table():
    try:
        table_name = read_while_not_set("Type the table's name: ", on_bad_input="Table name can't be empty")
        db_queries.show_table(table_name)
    except Exception:
        print("No such table")
        raise


@exception_handler(logger)
def get_sample():
    print("Input all desired arguments. Format: arg_name=arg_val:\n"
          "Possible parameters:\n"
          "patient_id, diagnosis, sex\n"
          "bdate_from, bdate_to, ethnicity\n"
          "t_since, t_until, result\n"
          "doctor_id, speciality, seniority\n"
          "position, columns_to_show")
    constraints = {}
    try:
        while True:
            line = input().strip()
            if line == '/cancel':
                return
            if not line:
                break
            words = line.split('=')
            if words[0] == 'columns_to_show':
                col_names = re.split('; |, |\*|\n', words[1])
                constraints['columns_to_show'] = col_names
            else:
                constraints[words[0]] = words[1]
        db_queries.get_sample_by_constraints(**constraints)
    except Exception as err:
        print(err)
        raise


if __name__ == '__main__':
    connection.initialize_connection()
    conn, cursor = connection.get_conn_cursor()

    while True:
        print("Type the next command: ", end='')
        command = input()
        if command == '/add_record':
            try:
                add_record()
            except Exception:
                pass
        elif command == '/update_record':
            try:
                update_record()
            except Exception:
                pass
        elif command == '/create_patient_profile':
            try:
                create_patient_profile()
            except Exception:
                pass
        elif command == '/update_patient_profile':
            try:
                update_patient_profile()
            except Exception:
                pass
        elif command == '/create_doctor_profile':
            try:
                create_doctor_profile()
            except Exception:
                pass
        elif command == '/update_doctor_profile':
            try:
                update_doctor_profile()
            except Exception:
                pass
        elif command == '/set_appointment':
            try:
                set_appointment()
            except Exception:
                pass
        elif command == '/patient_ids':
            try:
                get_patient_ids()
            except Exception:
                pass
        elif command == '/doctor_ids':
            try:
                get_doctor_ids()
            except Exception:
                pass
        elif command == '/patient_data':
           try:
               get_patient_data()
           except Exception:
               pass
        elif command == '/doctor_data':
            try:
                get_doctor_data()
            except Exception:
                pass
        elif command == '/patient_history':
            try:
                get_patient_history()
            except Exception:
                pass
        elif command == '/show_table':
            try:
                show_table()
            except Exception:
                pass
        elif command == '/get_sample':
            try:
                get_sample()
            except Exception:
                pass
        elif command == '/help':
            get_help()
        elif command == '/drop_table':
            try:
                cursor.execute("DROP SCHEMA IF EXISTS logs CASCADE;")
            except Exception:
                print("Execute failed: can't drop a table")
            try:
                conn.commit()
            except Exception:
                print("Commit failed: can't apply changes")
        elif command == '/restart_table':
            try:
                cursor.execute(open("DDL.sql", "r").read())
                os.system("fill_table.py")
            except Exception:
                print("Cannot restart table")
            try:
                conn.commit()
            except Exception:
                print("Commit failed: can't apply changes")
        elif command == '/exit':
            print("Session finished")
            break
        else:
            print('Unknown command')

    try:
        conn.commit()
    except Exception:
        print('Commit failed on exit')
    if conn:
        conn.close()
