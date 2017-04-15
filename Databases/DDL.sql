-- Schema: AnamnesisAutomated

DROP SCHEMA IF EXISTS logs CASCADE;

CREATE SCHEMA logs
  AUTHORIZATION postgres;

COMMENT ON SCHEMA logs
  IS 'MIPT DB CourseProject';

CREATE TABLE logs.patients (
    patient_id		SERIAL	NOT NULL,
    full_name		TEXT 	NOT NULL,
    sex			CHAR(1) CHECK(sex = 'M' OR sex = 'W'),
    date_of_birth	DATE,
    ethnicity		TEXT,
    relationship_status	TEXT,

    PRIMARY KEY (patient_id)
);

CREATE TABLE logs.contact_data (
    patient_id		SERIAL 	NOT NULL,
    address		TEXT,
    phone_number        TEXT,
    email		TEXT,
    
    FOREIGN KEY (patient_id) REFERENCES logs.patients (patient_id)
);

CREATE TABLE logs.drug_sensitivity (
    patient_id		SERIAL 	NOT NULL,
    drug_id		SERIAL 	NOT NULL,
    reaction		TEXT 	NOT NULL,
    
    FOREIGN KEY (patient_id) REFERENCES logs.patients (patient_id)
);

CREATE TABLE logs.doctors (
    doctor_id		SERIAL 	NOT NULL,
    full_name		TEXT	NOT NULL,
    degree		TEXT	NOT NULL,
    specialization	TEXT 	NOT NULL,
    seniority		INT,
    position		TEXT,
    salary 		NUMERIC	CHECK(salary > 0),

    PRIMARY KEY (doctor_id)
);

CREATE TABLE logs.doctor_sessions (
    doctor_id		SERIAL 	NOT NULL,
    patient_id		SERIAL 	NOT NULL,
    appointment		TIMESTAMP NOT NULL,
    reason		TEXT	NOT NULL,

    FOREIGN KEY (doctor_id) REFERENCES logs.doctors (doctor_id),
    FOREIGN KEY (patient_id) REFERENCES logs.patients (patient_id)
);

CREATE TABLE logs.medical_log (
    case_id		SERIAL 	NOT NULL,
    patient_id		SERIAL 	NOT NULL,
    therapist_id	SERIAL 	NOT NULL,
    entry_date		DATE,
    diagnosis		TEXT,
    treatment_result	TEXT,
    discharge_date	DATE 	CHECK (entry_date <= discharge_date),

    PRIMARY KEY (case_id),
    FOREIGN KEY (patient_id) REFERENCES logs.patients (patient_id),
    FOREIGN KEY (therapist_id) REFERENCES logs.doctors (doctor_id)
);

CREATE TABLE logs.conditions (
    case_id		SERIAL 	NOT NULL,
    description		TEXT 	NOT NULL,
    onset_date		DATE 	NOT NULL,
    
    FOREIGN KEY (case_id) REFERENCES logs.medical_log (case_id)
);

CREATE TABLE logs.drugs (
    drug_id		SERIAL 	NOT NULL,
    name		TEXT 	NOT NULL,
    type		TEXT 	NOT NULL, 
    price		NUMERIC NOT NULL CHECK (price > 0),
    
    PRIMARY KEY (drug_id)
);

CREATE TABLE logs.drug_indications (
    drug_id		SERIAL 	NOT NULL,
    prescription	TEXT 	NOT NULL,
    
    FOREIGN KEY (drug_id) REFERENCES logs.drugs (drug_id)
);

CREATE TABLE logs.drug_contraindications (
    drug_id		SERIAL 	NOT NULL,
    caution		TEXT 	NOT NULL,
    
    FOREIGN KEY (drug_id) REFERENCES logs.drugs (drug_id)
);

CREATE TABLE logs.drugs_bought (
    case_id		SERIAL 	NOT NULL,
    drug_id		SERIAL 	NOT NULL,
    purchase_date	DATE,
    
    FOREIGN KEY (case_id) REFERENCES logs.medical_log (case_id),
    FOREIGN KEY (drug_id) REFERENCES logs.drugs (drug_id)
);

CREATE TABLE logs.analyzes (
    analysis_id		SERIAL 	NOT NULL,
    description		TEXT 	NOT NULL,
    price		NUMERIC NOT NULL CHECK (price > 0),

    PRIMARY KEY(analysis_id)
);

CREATE TABLE logs.analysis_prescription (
    analysis_id		SERIAL 	NOT NULL,
    prescription	TEXT 	NOT NULL,

    FOREIGN KEY (analysis_id) REFERENCES logs.analyzes (analysis_id)
);

CREATE TABLE logs.analyzes_made (
    case_id		SERIAL 	NOT NULL,
    analysis_id		SERIAL 	NOT NULL,
    purchase_date	DATE,
    results		TEXT,
    
    FOREIGN KEY (case_id) REFERENCES logs.medical_log (case_id),
    FOREIGN KEY (analysis_id) REFERENCES logs.analyzes (analysis_id)
);

CREATE TABLE logs.surgeries (
    surgery_id		SERIAL 	NOT NULL,
    description		TEXT 	NOT NULL,
    price		NUMERIC NOT NULL CHECK(price > 0),

    PRIMARY KEY (surgery_id)
);

CREATE TABLE logs.surgery_indications (
    surgery_id		SERIAL 	NOT NULL,
    prescription	TEXT 	NOT NULL,
    
    FOREIGN KEY (surgery_id) REFERENCES logs.surgeries (surgery_id)
);

CREATE TABLE logs.surgery_contraindications (
    surgery_id		SERIAL 	NOT NULL,
    caution		TEXT 	NOT NULL,
    
    FOREIGN KEY (surgery_id) REFERENCES logs.surgeries (surgery_id)
);

CREATE TABLE logs.surgeries_performed (
    case_id		SERIAL 	NOT NULL,
    surgery_id		SERIAL 	NOT NULL,
    surgeon_id		SERIAL 	NOT NULL,
    operation_date	DATE 	NOT NULL,
    
    FOREIGN KEY (case_id) REFERENCES logs.medical_log (case_id),
    FOREIGN KEY (surgery_id) REFERENCES logs.surgeries (surgery_id),
    FOREIGN KEY (surgeon_id) REFERENCES logs.doctors (doctor_id)  
);
