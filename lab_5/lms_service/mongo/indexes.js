db = db.getSiblingDB('lms_mongo_db');

db.courses.createIndex({ teacher_id: 1 });
db.courses.createIndex({ title: 1 });
db.courses.createIndex({ "lessons.position": 1 });

print("Индексы созданы");