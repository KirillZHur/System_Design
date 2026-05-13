db = db.getSiblingDB('lms_mongo_db');

print(" READ: все курсы ");
db.courses.find().pretty();

print(" READ: курс по _id через $eq ");
db.courses.find({ _id: { $eq: NumberInt(1) } }).pretty();

print(" READ: курсы, где teacher_id != 1 ");
db.courses.find({ teacher_id: { $ne: NumberInt(1) } }).pretty();

print(" READ: курсы, где _id > 5 ");
db.courses.find({ _id: { $gt: NumberInt(5) } }).pretty();

print(" READ: курсы, где _id < 4 ");
db.courses.find({ _id: { $lt: NumberInt(4) } }).pretty();

print(" READ: курсы, где teacher_id входит в список ");
db.courses.find({
  teacher_id: { $in: [NumberInt(1), NumberInt(3), NumberInt(5)] }
}).pretty();

print(" READ: $and ");
db.courses.find({
  $and: [
    { teacher_id: { $eq: NumberInt(1) } },
    { _id: { $gt: NumberInt(3) } }
  ]
}).pretty();

print(" READ: $or ");
db.courses.find({
  $or: [
    { teacher_id: { $eq: NumberInt(2) } },
    { teacher_id: { $eq: NumberInt(4) } }
  ]
}).pretty();

print(" READ: курс по названию ");
db.courses.find({ title: "Основы программирования" }).pretty();

print(" READ: курс, у которого есть урок с position = 2 ");
db.courses.find({
  lessons: {
    $elemMatch: { position: NumberInt(2) }
  }
}).pretty();

print(" CREATE: добавить новый курс ");
db.courses.insertOne({
  _id: NumberInt(11),
  title: "DevOps",
  description: "Основы DevOps и CI/CD",
  teacher_id: NumberInt(2),
  created_at: ISODate("2026-04-11T10:00:00Z"),
  lessons: [
    {
      id: NumberInt(1),
      title: "Введение в DevOps",
      content: "Базовые понятия DevOps",
      position: NumberInt(1),
      created_at: ISODate("2026-04-11T10:10:00Z")
    }
  ]
});

print(" CREATE: добавить новый урок в курс через $push ");
db.courses.updateOne(
  { _id: NumberInt(1) },
  {
    $push: {
      lessons: {
        id: NumberInt(3),
        title: "Операторы",
        content: "Основные операторы языка",
        position: NumberInt(3),
        created_at: ISODate("2026-04-13T12:00:00Z")
      }
    }
  }
);

print(" UPDATE: изменить название курса ");
db.courses.updateOne(
  { _id: NumberInt(2) },
  {
    $set: {
      title: "Алгоритмы и структуры данных (обновленный курс)"
    }
  }
);

print(" UPDATE: изменить описание курса ");
db.courses.updateOne(
  { _id: NumberInt(3) },
  {
    $set: {
      description: "Обновленное описание курса по базам данных"
    }
  }
);

print(" UPDATE: изменить title урока с position = 2 в курсе _id = 1 ");
db.courses.updateOne(
  {
    _id: NumberInt(1),
    "lessons.position": NumberInt(2)
  },
  {
    $set: {
      "lessons.$.title": "Переменные и базовые типы данных"
    }
  }
);

print(" DELETE: удалить урок из курса через $pull ");
db.courses.updateOne(
  { _id: NumberInt(1) },
  {
    $pull: {
      lessons: { id: NumberInt(3) }
    }
  }
);

print(" DELETE: удалить курс ");
db.courses.deleteOne({ _id: NumberInt(11) });

print(" READ: итоговое состояние коллекции ");
db.courses.find().pretty();

print(" AGGREGATION: количество уроков в курсах преподавателя ");
db.courses.aggregate([
  {
    $match: {
      teacher_id: NumberInt(1)
    }
  },
  {
    $project: {
      _id: 1,
      title: 1,
      teacher_id: 1,
      lessons_count: { $size: "$lessons" }
    }
  },
  {
    $sort: {
      lessons_count: -1,
      title: 1
    }
  }
]).pretty();