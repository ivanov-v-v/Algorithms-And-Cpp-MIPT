import sys
import psycopg2
import my_logger

conn = None
cursor = None


@my_logger.exception_handler(my_logger.logger)
def initialize_connection():
    global conn
    global cursor
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


@my_logger.exception_handler(my_logger.logger)
def get_conn_cursor():
    return conn, cursor
