import psycopg2
import pandas as pd
import sys
import logging

# хорошая новость: с точки зрения реализации psycopg2
# эквивалентен обычным sql-запросам
# План:
# 1. Сгенерировать все .csv с данными
# 2. Собрать из него pandas.DataFrame
# 3. Датафреймы преобразовать в списки кортежей и загрузить в базу
# 4. Разобраться с назначением ролей пользователям
# 5. Добавить ограничения в DDL
# 6. Лучше продумать структуру генерируемых данных

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

    patients_df = pd.read_csv('../processed_data/patients.csv', sep='\t')
    patients = patients_df.to_records(index=False).tolist()

    patients_query = "INSERT INTO logs.patients (full_name, sex, date_of_birth, " \
                     "ethnicity, relationship_status, address, phone_number, email) " \
                     "VALUES (%s, %s, %s, %s, %s, %s, %s, %s)"

    cursor.executemany(patients_query, patients)
    conn.commit()

    # cursor.execute("SELECT * FROM logs.patients")
    # rows = cursor.fetchall()
    # for row in rows:
    #     print(row)

    doctors_df = pd.read_csv('../processed_data/doctors.csv', sep='\t')
    doctors = doctors_df.to_records(index=False).tolist()

    doctors_query = "INSERT INTO logs.doctors (full_name, degree, " \
                    "speciality, seniority, position, salary) " \
                    "VALUES (%s, %s, %s, %s, %s, %s)"

    cursor.executemany(doctors_query, doctors)
    conn.commit()

    # cursor.execute("SELECT * FROM logs.doctors")
    # rows = cursor.fetchall()
    # for row in rows:
    #     print(row)

    logs_df = pd.read_csv('../processed_data/medical_log.csv', sep='\t')
    logs = logs_df.to_records(index=False).tolist()

    logs_query = "INSERT INTO logs.medical_log (patient_id, therapist_id, " \
                 "entry_date, diagnosis, treatment_result, discharge_date)" \
                 " VALUES ((SELECT patient_id FROM logs.patients WHERE full_name=%s), " \
                 "(SELECT doctor_id FROM logs.doctors WHERE full_name=%s), " \
                 "%s, %s, %s, %s)"

    cursor.executemany(logs_query, logs)
    conn.commit()

    # cursor.execute("SELECT * FROM logs.medical_log")
    # rows = cursor.fetchall()
    # for row in rows:
    #     print(row)

    cursor.execute("DELETE FROM logs.medical_log")
    cursor.execute("DELETE FROM logs.patients")
    cursor.execute("DELETE FROM logs.doctors")
    conn.commit()
    if conn:
        conn.close()