import psycopg2
import sys

# Как в Питоне передать именованный список аргументов с произвольным числом параметров?

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


def read_and_process():
    word = input()
    if not word:
        return None
    return word


# добавить запись в больничный журнал
def add_record():
    print('patient full name: ', end='')
    patient_name = read_and_process()
    print('therapist full name: ', end='')
    therapist_name = read_and_process()
    print('entry date, YYYY-MM-DD: ', end='')
    entry_date = read_and_process()
    print('diagnosis: ', end='')
    diagnosis = read_and_process()
    print('treatment result: ', end='')
    treatment_result = read_and_process()
    print('discharge date, YYYY-MM-DD: ', end='')
    discharge_date = read_and_process()

    logs_query = "INSERT INTO logs.medical_log (patient_id, therapist_id, " \
                 "entry_date, diagnosis, treatment_result, discharge_date)" \
                 " VALUES ((SELECT patient_id FROM logs.patients WHERE full_name=%s), " \
                 "(SELECT doctor_id FROM logs.doctors WHERE full_name=%s), " \
                 "%s, %s, %s, %s)"

    cursor.execute(logs_query % (
        patient_name, therapist_name,
        entry_date, diagnosis,
        treatment_result, discharge_date
    ))
    conn.commit()
    print('Record added')
    return 0


# обновить запись в больничном журнале
def update_record(record_id):
    # нужно позаботиться о том, чтобы поля можно было обновлять по отдельности?
    print("new therapist's full name: ", end='')
    therapist_name = read_and_process()
    print('new diagnosis: ', end='')
    diagnosis = read_and_process()
    print('new treatment result: ', end='')
    treatment_result = read_and_process()
    print('new discharge date, YYYY-MM-DD: ', end='')
    discharge_date = read_and_process()
    return 0


# создать профиль пациента
def create_patient_profile():
    # по аналогии, куча ввода + запрос к базе
    return 0


# создать профиль сотрудника
def create_doctor_profile():
    # см. предыдущий
    return 0


# показать всю связанную информацию
def show_related_records(patient_id):
    # вывести все записи в больничном журнале,
    # связанные с этим пациентом, отсортированные
    # от новых к старым
    return 0


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
    finally:
        if conn:
            conn.close()
    while True:
        command = input()
        if command == '/add_record':
            add_record()
        elif command == '/update_record':
            update_record(-1)
        elif command == '/create_patient_profile':
            create_patient_profile()
        elif command == '/create_doctor_profile':
            create_doctor_profile()
        elif command == '/show_related_records':
            show_related_records(-1)
        elif command == '/exit':
            print("Session finished")
            break
        else:
            print('Unknown command')
