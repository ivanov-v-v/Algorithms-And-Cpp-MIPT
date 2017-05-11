import psycopg2
from psycopg2.extensions import AsIs
import pandas as pd
import datetime
import sys
import os
import functools
import logging

# TODO: Реализовать основные юзер сторис. Проблема: они недостаточно продуманы.
# В данный момент я не знаю, что именно реализовывать.
# Пользователи не должны напрямую взаимодействовать с SQL-запросами.
# 1. Добавить запись в больничный журнал
# 2. Создать анкету пациента
# 3. Показать, на основе имени, всю информацию, связанную с пациентом
# 4. Добавление сотрудников
# 5. Добавление лекарств и данных об индивидуальной чувствительности


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
def read_and_process(info_message):
    print(info_message, end='')
    word = input()
    if not word:
        return None
    word.strip()
    return word


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


@exception_handler(logger)
def valid_date(date_string):
    try:
        datetime.datetime.strptime(date_string, '%Y-%m-%d')
    except ValueError:
        return False
    return True


# добавить запись в больничный журнал
@exception_handler(logger)
def add_record():
    # TODO: используя обработку ошибок, написать здесь более симпатичный код
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

    while True:
        entry_date = read_and_process("Entry date, YYYY-MM-DD: ")
        if entry_date and not valid_date(entry_date):
            print("Invalid date format")
            entry_date = None
        else:
            break
    diagnosis = read_and_process('Diagnosis: ')
    treatment_result = read_and_process('Treatment result: ')
    while True:
        discharge_date = read_and_process('Discharge date, YYYY-MM-DD: ')
        if discharge_date and not valid_date(discharge_date):
            print("Invalid date format")
            discharge_date = None
        else:
            break

    logs_query = "INSERT INTO logs.medical_log (patient_id, therapist_id, " \
                 "entry_date, diagnosis, treatment_result, discharge_date)" \
                 " VALUES (%s, %s, %s, %s, %s, %s)"

    try:
        cursor.execute(logs_query, [
            patient_id, therapist_id,
            entry_date, diagnosis,
            treatment_result, discharge_date
        ])
    except Exception as err:
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
    # нужно позаботиться о том, чтобы поля можно было обновлять по отдельности?
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
    if new_discharge_date and not valid_date(new_discharge_date):
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
    # по аналогии, куча ввода + запрос к базе
    while True:
        patient_name = read_and_process("Patient's full name: ")
        if not patient_name:
            print("Name cannot be empty")
        else:
            break

    while True:
        patient_sex = read_and_process("Patient's biological sex: ")
        if patient_sex and (patient_sex != 'M' or patient_sex != 'F'):
            print("Invalid sex: M and F are allowed")
        else:
            break

    while True:
        date_of_birth = read_and_process("Date of birth, YYYY-MM-DD: ")
        if date_of_birth and not valid_date(date_of_birth):
            print("Invalid date format")
            date_of_birth = None
        else:
            break

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
    if new_date_of_birth and not valid_date(new_date_of_birth):
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
    while True:
        doctor_name = read_and_process("Doctor's full name: ")
        if not doctor_name:
            print("Name cannot be empty")
        else:
            break

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
def pretty_query_print(query, parameters_list):
    try:
        cursor.execute(query, parameters_list)
    except Exception as err:
        raise
    else:
        data = cursor.fetchall()
        colnames = [desc[0] for desc in cursor.description]
        df = pd.DataFrame(data, columns=colnames)
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
            print("Type the record's id: ", end='')
            record_id = input()
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
            print("Type the patient's id: ", end='')
            patient_id = input()
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
            print("Type the doctor's id: ", end='')
            doctor_id = input()
            if check_id_existence(table_name='logs.doctors', id_name='doctor_id', id_val=doctor_id):
                try:
                    update_doctor_profile(doctor_id)
                except:
                    print("Database error: can't update doctor's profile")
            else:
                print("No data about such a doctor's id")
        elif command == '/patient_data':
            print("Type the patient's id: ", end='')
            patient_id = input()
            if check_id_existence(table_name='logs.patients', id_name='patient_id', id_val=patient_id):
                try:
                    show_patient_personal_data(patient_id)
                except:
                    print("Database error: can't show patient's data")
            else:
                print("No data about such a patient's id")
        elif command == '/doctor_data':
            print("Type the doctor's id: ", end='')
            doctor_id = input()
            if check_id_existence(table_name='logs.doctors', id_name='doctor_id', id_val=doctor_id):
                try:
                    show_doctor_personal_data(doctor_id)
                except:
                    print("Database error: can't show doctor's data")
            else:
                print("No data about such a doctor's id")
        elif command == '/patient_history':
            print("Type the patient's id: ", end='')
            patient_id = input()
            if check_id_existence(table_name='logs.patients', id_name='patient_id', id_val=patient_id):
                try:
                    show_patient_history(patient_id)
                except:
                    print("Database error: can't show patient's history")
            else:
                print("No data about such a patient's id")
        elif command == '/show_table':
            print("Type the table's name: ", end='')
            table_name = input()
            try:
                show_table(table_name)
            except:
                print("No such table")
        elif command == '/drop_table':
            # TODO: добавить пользователей и сделать им разделение прав
            try:
                cursor.execute("DROP SCHEMA IF EXISTS logs CASCADE;")
            except Exception:
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
