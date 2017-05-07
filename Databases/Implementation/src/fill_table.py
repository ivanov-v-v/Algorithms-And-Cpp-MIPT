import psycopg2
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

conn = None
try:
    conn = psycopg2.connect(
        database='postgres',
        user='postgres',
        port='5432',
        password='',
        host='localhost'
    )
    cursor = conn.cursor()
    cursor.execute('SELECT version()')
    ver = cursor.fetchone()
    print(ver)
    cursor.execute("DELETE FROM logs.patients")
    conn.commit()
    cursor.execute("INSERT INTO logs.patients "
                   "(full_name, sex, ethnicity, relationship_status)"
                   "VALUES('Ivan Ivanov', 'M', 'Russian', 'Single')")
    conn.commit()
    cursor.execute("SELECT * FROM logs.patients")
    rows = cursor.fetchall()
    for row in rows:
        print(row)
    cursor.execute("DELETE FROM logs.patients")
    conn.commit()
except Exception as err:
    if conn:
        conn.rollback()
    print("Uh oh, can't connect")
    print(err)
finally:
    if conn:
        conn.close()