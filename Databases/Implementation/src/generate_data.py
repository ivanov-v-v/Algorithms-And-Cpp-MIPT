import pandas as pd
import numpy as np

patient_cols = ['full_name', 'sex', 'date_of_birth', ]
patients_df = pd.DataFrame({
    'full_name': [],
    'sex': [],
    'date_of_birth': [],
    'ethnicity': [],
    'relationship_status': [],
    'address': [],
    'phone_number': [],
    'email': [],
})

if __name__ == '__main__':
    patient = {}
    patient = {
        'full_name': 'Ivan Ivanov',
        'sex': 'M',
        'date_of_birth': '22-02-94',
        'ethnicity': 'Russian',
        'relationship_status': 'Single',
        'address': 'ahalai',
        'phone_number': 'mahalai',
        'email': 'random@random.com'
    }
    patients_df = patients_df.append(patient, ignore_index=True)
    print(patients_df)
