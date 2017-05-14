import logging


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
