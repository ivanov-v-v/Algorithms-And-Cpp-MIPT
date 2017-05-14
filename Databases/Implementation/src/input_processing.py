import datetime
import my_logger
from my_logger import exception_handler
from my_logger import logger


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
