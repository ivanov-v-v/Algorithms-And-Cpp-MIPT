import psycopg2
from psycopg2.extensions import AsIs
import pandas as pd
import datetime
import re

import my_logger
from my_logger import exception_handler
from my_logger import logger
import input_processing
from input_processing import read_and_process, read_while_not_set, read_while_not_valid
import connection


@exception_handler(logger)
def check_id_existence(**kwargs):
    conn, cursor = connection.get_conn_cursor()
    results = None
    try:
        cursor.execute(
            "SELECT * FROM %s WHERE %s=%s",
            [AsIs(kwargs['table_name']), AsIs(kwargs['id_name']), kwargs['id_val']]
        )
    except KeyError as k_err:
        print("{} is not in parameters list".format(k_err))
        return False
    except psycopg2.ProgrammingError as p_err:
        print("Invalid query: {}".format(str(p_err).strip()))
    else:
        results = cursor.fetchall()
    return False if not results else True


# добавить запись в больничный журнал
@exception_handler(logger)
def add_record():
    conn, cursor = connection.get_conn_cursor()
    # TODO: использя обработку ошибок, написать здесь более симпатичный код
    while True:
        patient_name = read_and_process("Patient's full name: ")
        if patient_name:
            cursor.execute(
                "SELECT patient_id FROM logs.patients WHERE patient_name=%s",
                [patient_name]
            )
            results = cursor.fetchall()
            if not results:
                print("Patient with such name is not found in database")
            else:
                patient_id = results[0][0]
                break
        else:
            print("Name cannot be empty")
    while True:
        doctor_name = read_and_process("Therapist's full name: ")
        if doctor_name:
            cursor.execute(
                "SELECT doctor_id FROM logs.doctors WHERE doctor_name=%s",
                [doctor_name]
            )
            results = cursor.fetchall()
            if not results:
                print("Doctor with such name is not found in database")
            else:
                doctor_id = results[0][0]
                break
        else:
            print("Name cannot be empty")

    entry_date = read_while_not_valid(
        "Entry date, YYYY-MM-DD: ", on_bad_input="Invalid date format",
        validator_function=input_processing.is_valid_date
    )
    diagnosis = read_and_process('Diagnosis: ')
    treatment_result = read_and_process('Treatment result: ')
    discharge_date = read_while_not_valid(
        "Discharge date, YYYY-MM-DD: ", on_bad_input="Invalid date format",
        validator_function=input_processing.is_valid_date
    )
    logs_query = "INSERT INTO logs.medical_log (patient_id, doctor_id, " \
                 "entry_date, diagnosis, treatment_result, discharge_date)" \
                 " VALUES (%s, %s, %s, %s, %s, %s)"

    try:
        cursor.execute(logs_query, [
            patient_id, doctor_id,
            entry_date, diagnosis,
            treatment_result, discharge_date
        ])
    except:
        conn.rollback()
        raise

    try:
        conn.commit()
    except:
        raise

    print('Record added')


# обновить запись в больничном журнале
@exception_handler(logger)
def update_record(record_id):
    conn, cursor = connection.get_conn_cursor()

    def set_record_fields(**kwargs):
        for field_name, value in kwargs.items():
            if not value:
                continue
            try:
                cursor.execute(
                    "UPDATE logs.medical_log "
                    "SET %s=%s"
                    "WHERE case_id=%s",
                    [AsIs(field_name), value, record_id]
                )
            except:
                conn.rollback()
                raise

    new_doctor_name = read_and_process("New doctor's full name: ")
    if new_doctor_name:
        try:
            cursor.execute(
                "SELECT doctor_id FROM logs.doctors WHERE doctor_name=%s",
                [new_doctor_name]
            )
        except:
            raise

        results = cursor.fetchall()
        if not results:
            print("Doctor with such name is not found in the database")
            new_doctor_id=None
        else:
            new_doctor_id = results[0][0]

    new_diagnosis = read_and_process('New diagnosis: ')
    new_treatment_result = read_and_process('New treatment result: ')
    new_discharge_date = read_and_process('New discharge date, YYYY-MM-DD: ')
    if new_discharge_date and not input_processing.is_valid_date(new_discharge_date):
        print("Invalid date format")
        new_discharge_date = None

    set_record_fields(
        doctor_id=new_doctor_id,
        diagnosis=new_diagnosis,
        treatment_result=new_treatment_result,
        new_discharge_date=new_discharge_date,
    )

    print("Record updated")
    try:
        conn.commit()
    except:
        raise


# создать профиль пациента
@exception_handler(logger)
def create_patient_profile():
    conn, cursor = connection.get_conn_cursor()
    patient_name = read_while_not_set("Patient's full name", on_bad_input="Name cannot be empty")
    patient_sex = read_while_not_valid(
        "Patient's biological sex: ",
        on_bad_input="Invalid sex: only M and F are allowed",
        validator_function=lambda ch: ch == 'M' or ch == 'F'
    )
    date_of_birth = read_while_not_valid(
        "Date of birth, YYYY-MM-DD: ",
        on_bad_input="Invalid date format",
        validator_function=input_processing.is_valid_date
    )
    ethnicity = read_and_process('Ethnicity: ')
    relationship_status = read_and_process('Relationship status: ')
    address = read_and_process('Home address: ')
    phone_number = read_and_process('Phone number: ')
    email = read_and_process("Email: ")

    logs_query = "INSERT INTO logs.patients (patient_name, sex, date_of_birth" \
                 "ethnicity, relationship_status, address, phone_number, email)" \
                 " VALUES (%s, %s, %s, %s, %s, %s, %s, %s)"

    try:
        cursor.execute(logs_query, [
            patient_name, patient_sex,
            date_of_birth, ethnicity,
            relationship_status, address,
            phone_number, email
        ])
    except:
        conn.rollback()
        raise

    try:
        conn.commit()
    except:
        raise

    print("Patient's personal data added")


@exception_handler(logger)
def update_patient_profile(patient_id):
    conn, cursor = connection.get_conn_cursor()

    def set_patient_fields(**kwargs):
        for field_name, value in kwargs.items():
            if not value:
                continue
            try:
                cursor.execute(
                    "UPDATE logs.patients "
                    "SET %s=%s "
                    "WHERE patient_id=%s",
                    [AsIs(field_name), value, patient_id]
                )
            except Exception as err:
                conn.rollback()
                raise

    new_patient_name = read_and_process("New patient's full name: ")
    new_sex = read_and_process("New patient's biological sex: ")
    new_date_of_birth = read_and_process('New date of birth, YYYY-MM-DD: ')
    if new_date_of_birth and not input_processing.is_valid_date(new_date_of_birth):
        print("Invalid date format")
        new_date_of_birth = None
    new_ethnicity = read_and_process("New ethnicity data: ")
    new_relationship_status = read_and_process("New relationship status: ")
    new_address = read_and_process("New home address: ")
    new_phone_number = read_and_process("New phone number: ")
    new_email = read_and_process("New email: ")

    set_patient_fields(
        patient_name=new_patient_name,
        sex=new_sex,
        date_of_birth=new_date_of_birth,
        relationship_status=new_relationship_status,
        ethnicity=new_ethnicity,
        address=new_address,
        phone_number=new_phone_number,
        email=new_email
    )

    print("Personal data updated")
    try:
        conn.commit()
    except:
        raise


# создать профиль сотрудника
@exception_handler(logger)
def create_doctor_profile():
    conn, cursor = connection.get_conn_cursor()
    # см. предыдущий
    doctor_name = read_while_not_set("Doctor's full name:", on_bad_input="Name cannot be empty")
    degree = read_and_process('Highest degree obtained: ')
    speciality = read_and_process('Main speciality: ')
    seniority = read_and_process('Seniority, years: ')
    position = read_and_process('Current position: ')
    salary = read_and_process("Salary, $: ")

    logs_query = "INSERT INTO logs.doctors (doctor_name, degree, " \
                 "speciality, seniority, position, salary)" \
                 " VALUES (%s, %s, %s, %s, %s, %s)"

    try:
        cursor.execute(logs_query, [
            doctor_name, degree,
            speciality, seniority,
            position, salary
        ])
    except:
        conn.rollback()
        raise

    try:
        conn.commit()
    except:
        raise

    print("Doctors's personal data added")


@exception_handler(logger)
def update_doctor_profile(doctor_id):
    conn, cursor = connection.get_conn_cursor()

    def set_doctor_fields(**kwargs):
        for field_name, value in kwargs.items():
            if not value:
                continue
            try:
                cursor.execute(
                    "UPDATE logs.doctors "
                    "SET %s=%s "
                    "WHERE doctor_id=%s",
                    [AsIs(field_name), value, doctor_id]
                )
            except Exception as err:
                conn.rollback()
                raise

    new_doctor_name = read_and_process("New full name: ")
    new_degree = read_and_process("New degree: ")
    new_speciality = read_and_process("New speciality: ")
    new_seniority = read_and_process("New seniority, years: ")
    new_position = read_and_process("New position: ")
    new_salary = read_and_process("New salary, $: ")

    set_doctor_fields(
        doctor_name=new_doctor_name,
        degree=new_degree,
        speciality=new_speciality,
        seniority=new_seniority,
        position=new_position,
        salary=new_salary
    )

    print("Personal data updated")
    try:
        conn.commit()
    except:
        raise


@exception_handler(logger)
def add_session(doctor_id, patient_id):
    conn, cursor = connection.get_conn_cursor()
    appointment_date = read_while_not_valid(
        "Desired appointment date, YYYY-MM-DD: ",
        on_bad_input="Date must be set",
        validator_function=input_processing.is_valid_date
    )
    reason = read_and_process("Reason for seeking medical attention: ")
    try:
        cursor.execute(
            "INSERT INTO logs.doctor_sessions "
            "(doctor_id, patient_id, appointment, reason) "
            "VALUES (%s, %s, %s, %s)",
            [doctor_id, patient_id, appointment_date, reason]
        )
    except:
        conn.rollback()
        raise

    try:
        conn.commit()
    except:
        raise
    print("Session added")


@exception_handler(logger)
def pretty_query_print(query, parameters_list, column_names=None):
    conn, cursor = connection.get_conn_cursor()
    try:
        cursor.execute(query, parameters_list)
    except:
        conn.rollback()
        raise
    else:
        data = cursor.fetchall()
        all_cols = [desc[0] for desc in cursor.description]
        df = pd.DataFrame(data, columns=all_cols)
        if column_names:
            df = df[column_names]
        print(df.to_string(), end='\n\n')


@exception_handler(logger)
def show_patient_personal_data(patient_id):
    try:
        pretty_query_print(
            "SELECT * FROM logs.patients WHERE patient_id=%s",
            [patient_id]
        )
    except:
        raise


@exception_handler(logger)
def show_doctor_personal_data(doctor_id):
    try:
        pretty_query_print(
            "SELECT * FROM logs.doctors WHERE doctor_id=%s",
            [doctor_id]
        )
    except:
        raise


@exception_handler(logger)
def get_ids_by_pname(patient_name):
    try:
        pretty_query_print(
            "SELECT patient_id FROM logs.patients WHERE patient_name=%s",
            [patient_name]
        )
    except:
        raise


@exception_handler(logger)
def get_ids_by_dname(doctor_name):
    try:
        pretty_query_print(
            "SELECT doctor_id FROM logs.doctors WHERE doctor_name=%s",
            [doctor_name]
        )
    except:
        raise


# показать всю связанную информацию
@exception_handler(logger)
def show_patient_history(patient_id):
    # вывести все записи в больничном журнале,
    # связанные с этим пациентом, отсортированные
    # от новых к старым
    try:
        pretty_query_print(
            "SELECT * FROM logs.patients WHERE patient_id=%s",
            [patient_id]
        )
        pretty_query_print(
            "SELECT * FROM logs.medical_log WHERE patient_id=%s",
            [patient_id]
        )
    except:
        raise


@exception_handler(logger)
def show_table(table_name):
    try:
        pretty_query_print(
            "SELECT * FROM %s",
            [AsIs(table_name)]
        )
    except:
        raise


@exception_handler(logger)
def get_sample_by_constraints(**kwargs):
    def getter(my_dict, key, default_val=None):
        return my_dict[key] if key in my_dict.keys() else default_val

    target_patient_id = getter(kwargs, 'patient_id')
    target_diagnosis = getter(kwargs, 'diagnosis')
    target_sex = getter(kwargs, 'sex')
    target_bdate_from = getter(kwargs, 'bdate_from', default_val=str(datetime.datetime(1000, 1, 1)))
    target_bdate_to = getter(kwargs, 'bdate_to', default_val=datetime.datetime.now())
    target_ethnicity = getter(kwargs, 'ethnicity')
    target_hospitalized_since = getter(kwargs, 't_since', default_val=datetime.datetime(1000, 1, 1))
    target_hospitalized_until = getter(kwargs, 't_until', default_val=datetime.datetime.now())
    target_result = getter(kwargs, 'result')

    target_doctor_id = getter(kwargs, 'doctor_id')
    target_speciality = getter(kwargs, 'speciality')
    target_seniority = getter(kwargs, 'seniority')
    target_position = getter(kwargs, 'position')

    target_columns = getter(kwargs, 'columns_to_show')

    raw_query = str(cursor.mogrify(
        "SELECT * "
        "FROM (logs.patients NATURAL JOIN logs.medical_log NATURAL JOIN logs.doctors) "
        "WHERE patient_id=%s "
        "AND diagnosis=%s "
        "AND date_of_birth BETWEEN %s AND %s "
        "AND sex=%s "
        "AND ethnicity=%s "
        "AND treatment_result=%s "
        "AND doctor_id=%s "
        "AND speciality=%s "
        "AND seniority=%s "
        "AND position=%s "
        "AND entry_date BETWEEN %s AND %s",
        [target_patient_id, target_diagnosis,
         target_bdate_from, target_bdate_to,
         target_sex, target_ethnicity, target_result,
         target_doctor_id, target_speciality,
         target_seniority, target_position,
         target_hospitalized_since, target_hospitalized_until]
    ))[2:-1]

    query = re.sub("=NULL", " IS NOT NULL", raw_query)
    try:
        pretty_query_print(query, (None,), column_names=target_columns)
    except:
        raise
