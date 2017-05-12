import psycopg2
from psycopg2.extensions import AsIs
import pandas as pd
import datetime
import sys
import os
import functools
import re
import logging


# TODO: Реализовать основные юзер сторис. Проблема: они недостаточно продуманы.
'''
1. Добавить запись в больничный журнал
2. Создать анкету пациента
3. Показать, на основе имени, всю информацию, связанную с пациентом
4. Добавление сотрудников
5. Добавление лекарств и данных об индивидуальной чувствительности
6. Удаление записей
7. На роль фармацевта придётся забить
8. На инспектора тоже придётся забить
   (т.к. нужны данные по анализам, лекарствам, их назначениям,
   и, что самое неприятное, их сопоставлению с списком показаний)
9. Можно реализовать исследователя, т.к. генерацию выборки можно производить
   средствами SQL, а дальше — анализировать с помощью pandas, т.к. там интуитивно
   понятный API
10. Реализовать добавление индивидуальных особенностей
11. Выкачать базу названий лекарств, заполнить оставшиеся таблицы
12. Реализовать запись к врачу
'''


conn = None
cursor = None


def create_logger():
    """
    Creates a logging object and returns it
    """
    logger = logging.getLogger("example_logger")
    logger.setLevel(logging.INFO)

    # create the logging file handler
    fh = logging.FileHandler("service.log", 'w')

    fmt = '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    formatter = logging.Formatter(fmt)
    fh.setFormatter(formatter)

    # add handler to logger object
    logger.addHandler(fh)
    return logger

logger = create_logger()


def exception_handler(logger):
    """
    A decorator that wraps the passed in function and logs
    exceptions should one occur

    @param logger: The logging object
    """
    def decorator(func):
        def wrapper(*args, **kwargs):
            try:
                return func(*args, **kwargs)
            except Exception as err:
                # log the exception
                err = "There was an exception in  "
                err += func.__name__
                logger.exception(err)
                # re-raise the exception
                raise
        return wrapper
    return decorator



@exception_handler(logger)
def is_valid_date(date_string):
    if not date_string:
        return True
    try:
        datetime.datetime.strptime(date_string, '%Y-%m-%d')
    except ValueError:
        return False
    return True


@exception_handler(logger)
def read_and_process(info_message):
    print(info_message, end='')
    word = input()
    if not word:
        return None
    word.strip()
    # TODO: создать именованный класс для этой ошибки
    if word == '/cancel':
        raise
    return word


@exception_handler(logger)
def read_while_not_set(info_message, on_bad_input):
    while True:
        return_val = read_and_process(info_message)
        if not return_val:
            print(on_bad_input)
        else:
            break
    return return_val


@exception_handler(logger)
def read_while_not_valid(info_message, on_bad_input, validator_function):
    return_val = None
    while True:
        return_val = read_and_process(info_message)
        if return_val and not validator_function(return_val):
            print(on_bad_input)
            return_val = None
        else:
            break
    return return_val


@exception_handler(logger)
def check_id_existence(**kwargs):
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
    # TODO: использя обработку ошибок, написать здесь более симпатичный код
    while True:
        patient_name = read_and_process("Patient's full name: ")
        if patient_name:
            cursor.execute(
                "SELECT patient_id FROM logs.patients WHERE full_name=%s",
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
        therapist_name = read_and_process("Therapist's full name: ")
        if therapist_name:
            cursor.execute(
                "SELECT doctor_id FROM logs.doctors WHERE full_name=%s",
                [therapist_name]
            )
            results = cursor.fetchall()
            if not results:
                print("Doctor with such name is not found in database")
            else:
                therapist_id = results[0][0]
                break
        else:
            print("Name cannot be empty")

    entry_date = read_while_not_valid(
        "Entry date, YYYY-MM-DD: ", on_bad_input="Invalid date format",
        validator_function=is_valid_date
    )
    diagnosis = read_and_process('Diagnosis: ')
    treatment_result = read_and_process('Treatment result: ')
    discharge_date = read_while_not_valid(
        "Discharge date, YYYY-MM-DD: ", on_bad_input="Invalid date format",
        validator_function=is_valid_date
    )
    logs_query = "INSERT INTO logs.medical_log (patient_id, therapist_id, " \
                 "entry_date, diagnosis, treatment_result, discharge_date)" \
                 " VALUES (%s, %s, %s, %s, %s, %s)"

    try:
        cursor.execute(logs_query, [
            patient_id, therapist_id,
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

    new_therapist_name = read_and_process("New therapist's full name: ")
    if new_therapist_name:
        try:
            cursor.execute(
                "SELECT doctor_id FROM logs.doctors WHERE full_name=%s",
                [new_therapist_name]
            )
        except:
            raise

        results = cursor.fetchall()
        if not results:
            print("Doctor with such name is not found in the database")
            new_therapist_id=None
        else:
            new_therapist_id = results[0][0]

    new_diagnosis = read_and_process('New diagnosis: ')
    new_treatment_result = read_and_process('New treatment result: ')
    new_discharge_date = read_and_process('New discharge date, YYYY-MM-DD: ')
    if new_discharge_date and not is_valid_date(new_discharge_date):
        print("Invalid date format")
        new_discharge_date = None

    set_record_fields(
        therapist_id=new_therapist_id,
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
    patient_name = read_while_not_set("Patient's full name", on_bad_input="Name cannot be empty")
    patient_sex = read_while_not_valid(
        "Patient's biological sex: ",
        on_bad_input="Invalid sex: only M and F are allowed",
        validator_function=lambda ch: ch == 'M' or ch == 'F'
    )
    date_of_birth = read_while_not_valid(
        "Date of birth, YYYY-MM-DD: ",
        on_bad_input="Invalid date format",
        validator_function=is_valid_date
    )
    ethnicity = read_and_process('Ethnicity: ')
    relationship_status = read_and_process('Relationship status: ')
    address = read_and_process('Home address: ')
    phone_number = read_and_process('Phone number: ')
    email = read_and_process("Email: ")

    logs_query = "INSERT INTO logs.patients (full_name, sex, date_of_birth" \
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
    if new_date_of_birth and not is_valid_date(new_date_of_birth):
        print("Invalid date format")
        new_date_of_birth = None
    new_ethnicity = read_and_process("New ethnicity data: ")
    new_relationship_status = read_and_process("New relationship status: ")
    new_address = read_and_process("New home address: ")
    new_phone_number = read_and_process("New phone number: ")
    new_email = read_and_process("New email: ")

    set_patient_fields(
        full_name=new_patient_name,
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
    # см. предыдущий
    doctor_name = read_while_not_set("Doctor's full name:", on_bad_input="Name cannot be empty")
    degree = read_and_process('Highest degree obtained: ')
    speciality = read_and_process('Main speciality: ')
    seniority = read_and_process('Seniority, years: ')
    position = read_and_process('Current position: ')
    salary = read_and_process("Salary, $: ")

    logs_query = "INSERT INTO logs.doctors (full_name, degree, " \
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
        full_name=new_doctor_name,
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
    appointment_date = read_while_not_valid(
        "Desired appointment date, YYYY-MM-DD: ",
        on_bad_input="Date must be set", validator_function=is_valid_date
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
def add_condition(case_id):
    description = read_while_not_set("Description: ", on_bad_input="Description can't be empty")
    onset_date = read_while_not_set("Onset date, YY-MM-DD: ", on_bad_input="Onset date must be set")


@exception_handler(logger)
def pretty_query_print(query, parameters_list, column_names=None):
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
        "Queries and statistics:\n"
        "|----/show_table: show table with given name\n"
        "|----/get_sample: obtain all data satisfying given constraints\n"
        "Service:\n"
        "|----/help: show this help page\n"
        "|----/cancel: interrupt current command and choose another\n"
        "|----/exit: close database"
    )


if __name__ == '__main__':
    conn = None
    cursor = None
    try:
        conn = psycopg2.connect(
            database='postgres',
            user='postgres',
            port='5432',
            password='',
            host='localhost'
        )
        cursor = conn.cursor()
    except Exception as err:
        if conn:
            conn.rollback()
        print(err)
        sys.exit()

    while True:
        print("Type the next command: ", end='')
        command = input()
        if command == '/add_record':
            try:
                add_record()
            except:
                print("Database error: can't add patient's record")
        elif command == '/update_record':
            try:
                record_id = read_while_not_set("Record's id: ", on_bad_input="Id must be set")
            except:
                continue
            if check_id_existence(table_name='logs.medical_log', id_name='case_id', id_val=record_id):
                try:
                    update_record(record_id)
                except:
                    print("Database error: can't update patient's record")
            else:
                print("No data about such a patient's id")
        elif command == '/create_patient_profile':
            try:
                create_patient_profile()
            except:
                print("Database error: can't create patient's profile")
        elif command == '/update_patient_profile':
            try:
                patient_id = read_while_not_set("Patient's id: ", on_bad_input="Id must be set")
            except:
                continue
            if check_id_existence(table_name='logs.patients', id_name='patient_id', id_val=patient_id):
                try:
                    update_patient_profile(patient_id)
                except:
                    print("Database error: can't update patient's profile")
            else:
                print("No data about such a patient's id")
        elif command == '/create_doctor_profile':
            try:
                create_doctor_profile()
            except:
                print("Database error: can't create doctor's profile")
        elif command == '/update_doctor_profile':
            try:
                doctor_id = read_while_not_set("Doctor's id: ", on_bad_input="Id must be set")
            except:
                continue
            if check_id_existence(table_name='logs.doctors', id_name='doctor_id', id_val=doctor_id):
                try:
                    update_doctor_profile(doctor_id)
                except:
                    print("Database error: can't update doctor's profile")
            else:
                print("No data about such a doctor's id")
        elif command == '/set_appointment':
            try:
                patient_id = read_while_not_set("Patient's id: ", on_bad_input="Id must be set")
                doctor_id = read_while_not_set("Doctor's id: ", on_bad_input="Id must be set")
                add_session(patient_id, doctor_id)
            except:
                print("Database error: can't add a session")
        elif command == '/patient_ids':
            try:
                patient_name = read_while_not_set("Patient's name: ", on_bad_input="Patient's name can't be empty")
                get_ids_by_pname(patient_name)
            except:
                print("Can't get the ids")
                continue
        elif command == '/doctor_ids':
            try:
                doctor_name = read_while_not_set("Doctor's name: ", on_bad_input="Doctor's name can't be empty")
                get_ids_by_dname(doctor_name)
            except:
                print("Can't get the ids")
                continue
        elif command == '/patient_data':
            try:
                patient_id = read_while_not_set("Patient's id: ", on_bad_input="Id must be set")
            except:
                continue
            if check_id_existence(table_name='logs.patients', id_name='patient_id', id_val=patient_id):
                try:
                    show_patient_personal_data(patient_id)
                except:
                    print("Database error: can't show patient's data")
            else:
                print("No data about such a patient's id")
        elif command == '/doctor_data':
            try:
                doctor_id = read_and_process("Type the doctor's id: ")
            except:
                continue
            if check_id_existence(table_name='logs.doctors', id_name='doctor_id', id_val=doctor_id):
                try:
                    show_doctor_personal_data(doctor_id)
                except:
                    print("Database error: can't show doctor's data")
            else:
                print("No data about such a doctor's id")
        elif command == '/patient_history':
            try:
                doctor_id = read_while_not_set("Doctor's id: ", on_bad_input="Id must be set")
            except:
                continue
            if check_id_existence(table_name='logs.patients', id_name='patient_id', id_val=patient_id):
                try:
                    show_patient_history(patient_id)
                except:
                    print("Database error: can't show patient's history")
            else:
                print("No data about such a patient's id")
        elif command == '/show_table':
            try:
                table_name = read_while_not_set("Type the table's name: ", on_bad_input="Table name can't be empty")
                show_table(table_name)
            except:
                print("No such table")
        elif command == '/get_sample':
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
                        raise
                    if not line:
                        break
                    words = line.split('=')
                    if words[0] == 'columns_to_show':
                        col_names = re.split('; |, |\*|\n', words[1])
                        constraints['columns_to_show'] = col_names
                    else:
                        constraints[words[0]] = words[1]
                get_sample_by_constraints(**constraints)
            except Exception as err:
                print(err)
                continue
        elif command == '/help':
            get_help()
        elif command == '/drop_table':
            try:
                cursor.execute("DROP SCHEMA IF EXISTS logs CASCADE;")
            except:
                print("Execute failed: can't drop a table")
            try:
                conn.commit()
            except:
                print("Commit failed: can't apply changes")
        elif command == '/restart_table':
            try:
                cursor.execute(open("DDL.sql", "r").read())
                os.system("fill_table.py")
            except:
                print("Cannot restart table")
            try:
                conn.commit()
            except:
                print("Commit failed: can't apply changes")
        elif command == '/exit':
            print("Session finished")
            break
        else:
            print('Unknown command')

    try:
        conn.commit()
    except:
        print('Commit failed on exit')
    if conn:
        conn.close()
