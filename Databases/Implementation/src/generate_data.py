import csv
import datetime
import random
import string

from random import randint

PATIENTS_CNT = 100
DOCTORS_CNT = 100


class Patient:
    def __init__(self, patient_name, sex, date_of_birth,
                 ethnicity, relationship_status, address,
                 phone_number, email, diagnosis_t):
        self.patient_name = patient_name
        self.sex = sex
        self.date_of_birth = date_of_birth
        self.ethnicity = ethnicity
        self.relationship_status = relationship_status
        self.address = address
        self.phone_number = phone_number
        self.email = email
        self.diagnosis_t = diagnosis_t


class Doctor:
    def __init__(self, doctor_name, degree, speciality,
                 seniority, position, salary):
        self.doctor_name = doctor_name
        self.degree = degree
        self.speciality = speciality
        self.seniority = seniority
        self.position = position
        self.salary = salary


male_names = []
female_names = []
surnames = []
rel_statuses = []
ethnicities = []
specialities = []
degrees = []
positions = {}
diseases = {}

patients = []
doctors = []
sick_leaves = {}


def file_to_list(filename, reslist):
    with open(filename, 'r') as readf:
        for line in readf:
            line = line.strip('\n')
            if not line:
                continue
            reslist.append(line)


def upload_raw_data():
    file_to_list('../raw_data/male_names.txt', male_names)
    file_to_list('../raw_data/female_names.txt', female_names)
    file_to_list('../raw_data/surnames.txt', surnames)
    file_to_list('../raw_data/nationalities.txt', ethnicities)
    file_to_list('../raw_data/relationship_statuses.txt', rel_statuses)
    file_to_list('../raw_data/degrees.txt', degrees)
    with open('../raw_data/positions.txt') as readf:
        degree = None
        for line in readf:
            line = line.strip('\n')
            if not line:
                continue
            if line[-1] == ':':
                degree = line[:-1]
                degrees.append(degree)
                positions[degree] = []
            else:
                positions[degree].append(line)
    with open('../raw_data/disease_by_type.txt') as readf:
        d_type = None
        for line in readf:
            line = line.strip('\n')
            if not line:
                continue
            if line[-1] == ':':
                d_type = line[:-1]
                specialities.append(d_type)
                diseases[d_type] = []
            else:
                diseases[d_type].append(line)

def generate_drugs():
    return 0


def generate_patients():
    with open('../processed_data/patients.csv', 'w') as pcsv:
        filewriter = csv.writer(
            pcsv,
            delimiter='\t',
            quotechar='"',
            quoting=csv.QUOTE_MINIMAL
        )
        filewriter.writerow(['patient_name', 'sex', 'date_of_birth', 'ethnicity',
                             'relationship_status', 'address', 'phone_number', 'email'])
        for it in range(PATIENTS_CNT):
            sex = 'M' if randint(1, 10**9 + 1) % 2 else 'F'
            name = random.choice(male_names if sex == 'M' else female_names)
            surname = random.choice(surnames)
            patient_name = '{} {}'.format(name, surname)
            date_of_birth = datetime.date(1950 + randint(0, 55), randint(1, 12), randint(1, 28))
            ethnicity = random.choice(ethnicities)
            relationship_status = random.choice(rel_statuses)
            address = '{} str. {}'.format(random.choice(surnames), randint(1, 1000))
            phone_number = '{}'.format(''.join(random.choice(string.digits) for _ in range(11)))
            email = '{}@gmail.com'.format(''.join(patient_name.split()))
            for obj in patients:
                if obj.patient_name == patient_name:
                    it -= 1
                    continue
            filewriter.writerow([patient_name, sex, date_of_birth, ethnicity, relationship_status, address, phone_number, email])
            diagnosis_t = random.choice(specialities)
            patients.append(
                Patient(
                    patient_name, sex, date_of_birth,
                    ethnicity, relationship_status,
                    address, phone_number, email, diagnosis_t
                )
            )


def generate_doctors():
    def get_rand_seniority(deg):
        return {
            deg == 'DM': randint(10, 40),
            deg == 'PhD': randint(5, 20),
            deg == 'MSc': randint(2, 7),
            deg == 'MBBS': randint(0, 3)
        }[True]

    def get_rand_salary(deg):
        return {
            deg == 'DM': randint(6000, 20000),
            deg == 'PhD': randint(2000, 7000),
            deg == 'MSc': randint(1500, 3000),
            deg == 'MBBS': randint(500, 2000)
        }[True]

    with open('../processed_data/doctors.csv', 'w') as dcsv:
        filewriter = csv.writer(
            dcsv,
            delimiter='\t',
            quotechar='"',
            quoting=csv.QUOTE_MINIMAL
        )
        filewriter.writerow(['doctor_name', 'degree', 'specialization', 'seniority', 'position', 'salary'])
        for it in range(DOCTORS_CNT):
            sex = 'M' if randint(1, 10**9 + 1) % 2 else 'F'
            name = random.choice(male_names if sex == 'M' else female_names)
            surname = random.choice(surnames)
            doctor_name = '{} {}'.format(name, surname)
            degree = random.choice(degrees)
            speciality = random.choice(specialities)
            seniority = get_rand_seniority(degree)
            position = random.choice(positions[degree])
            salary = get_rand_salary(degree)
            for obj in doctors:
                if obj.doctor_name == doctor_name:
                    it -= 1
                    continue
            filewriter.writerow([doctor_name, degree, speciality, seniority, position, salary])
            doctors.append(
                Doctor(
                    doctor_name, degree, speciality,
                    seniority, position, salary
                )
            )


def generate_logs():
    with open('../processed_data/medical_log.csv', 'w') as lcsv:
        filewriter = csv.writer(
            lcsv,
            delimiter='\t',
            quotechar='"',
            quoting=csv.QUOTE_MINIMAL
        )
        filewriter.writerow(['patient_name', 'doctor_name',
                             'entry_date', 'diagnosis', 'treatment_result',
                             'discharge_date'])
        for it in range(PATIENTS_CNT):
            patient = patients[it]
            diagnosis = random.choice(diseases[patient.diagnosis_t])
            specs_of_interest = list(filter(lambda x: x.speciality == patient.diagnosis_t, doctors))
            therapist = random.choice(specs_of_interest)
            entry_date = datetime.datetime.now().date() - datetime.timedelta(days=randint(1000, 10000))
            discharge_date = datetime.datetime.now().date() - datetime.timedelta(days=randint(10, 100))
            treatment_result = 'Died' if randint(1, 10**9) < 25865000 else 'Cured'
            filewriter.writerow(
                [patient.patient_name, therapist.doctor_name,
                 entry_date, diagnosis, treatment_result, discharge_date]
            )
            if patient.patient_name in sick_leaves.keys():
                sick_leaves[patient.patient_name].append((entry_date, discharge_date))
            else:
                sick_leaves.update({patient.patient_name: (entry_date, discharge_date)})


if __name__ == '__main__':
    upload_raw_data()
    generate_patients()
    generate_doctors()
    generate_logs()