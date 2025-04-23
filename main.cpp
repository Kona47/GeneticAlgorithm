#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>

using namespace std;

//Facilitator and Time dont need structs

struct Activity{
    string name;
    int expEnrollment;
    vector<string> preferredFacilitators;
    vector<string> otherFacilitators;
};

struct Room{
    string name;
    int capacity;
};

struct ScheduleItem{
    Activity activity;
    Room room;
    string facilitator;
    string time;
};

//A full schedule
struct Schedule{
    vector<ScheduleItem> schedule;
};

//All possibilities
vector<string> facilitators = {"Lock", "Glen", "Banks", "Richards", "Shaw", "Singer", "Uther", "Tyler", "Numen", "Zeldin"};

vector<Room> rooms = {{"Slater 003", 45}, {"Roman 216", 30}, {"Loft 206", 75},
                      {"Roman 201", 50}, {"Loft 310", 108}, {"Beach 201", 60}, {"Beach 301", 75}};

vector<Activity> activities = {{"SLA101A", 50, {"Glen", "Lock", "Banks", "Zeldin"}, {"Numen", "Richards"}},
                            {"SLA101B", 50, {"Glen", "Lock", "Banks", "Zeldin"}, {"Numen", "Richards"}},
                            {"SLA191A", 50, {"Glen", "Lock", "Banks", "Zeldin"}, {"Numen", "Richards"}},
                            {"SLA191B", 50, {"Glen", "Lock", "Banks", "Zeldin"}, {"Numen", "Richards"}},
                            {"SLA201", 50, {"Glen", "Banks", "Zeldin", "Shaw"}, {"Numen", "Richards", "Singer"}},
                            {"SLA291", 50, {"Lock", "Banks", "Zeldin", "Singer"}, {"Numen", "Richards", "Shaw", "Tyler"}},
                            {"SLA303", 60, {"Glen", "Zeldin", "Banks"}, {"Numen", "Singer", "Shaw"}},
                            {"SLA304", 25, {"Glen", "Banks", "Tyler"}, {"Numen", "Singer", "Shaw", "Richards", "Uther", "Zeldin"}},
                            {"SLA394", 20, {"Tyler", "Singer"}, {"Richards", "Zeldin"}},
                            {"SLA449", 60, {"Tyler", "Singer", "Shaw"}, {"Zeldin", "Uther"}},
                            {"SLA451", 100, {"Tyler", "Singer", "Shaw"}, {"Zeldin", "Uther", "Richards", "Banks"}}};

vector<string> times = {"10 AM", "11 AM", "12 PM", "1 PM", "2 PM", "3 PM"};


//random
mt19937 rng(random_device{}());

//Get a random element from
template <typename T>
T getRandomElement(const std::vector<T>& list) {
    uniform_int_distribution<> dist(0, list.size() - 1);
    return list[dist(rng)];
}

//Generate a schedule
Schedule generateRandomSchedule(){
    Schedule s;

    for(auto &activity : activities){
        ScheduleItem item;
        item.activity = activity;
        item.room = getRandomElement(rooms);
        item.facilitator = getRandomElement(facilitators);
        item.time = getRandomElement(times);
        s.schedule.push_back(item);
    }
    return s;
}

/*void displaySchedule(Schedule &s){
    for(auto &item : s.schedule){
        cout << item.room.name << ", " << item.activity.name << ", " << item.facilitator << ", " << item.time << '\n';
    }
    cout << "Score: " << s.fitness;
}*/

//functions for Checking fitness scores

//Activity fitness Scoring
double calcActivityScore(Schedule &s){
    double score = 0.0;

    //Deduct for activities at same room & time
    for(int i = 0; i < s.schedule.size(); i++){
        for(int j = i+1; j < s.schedule.size(); j++){
            if(s.schedule[i].room.name == s.schedule[j].room.name 
            && s.schedule[i].time == s.schedule[j].time)
                score -= 0.5;
        }

    }

    //Update score based on preffered/other facilitator
    for(auto &item : s.schedule){
        Activity a = item.activity;
        string facilitator = item.facilitator;
        bool nope = false;
        //Check if facilitator was a preferred one
        for(auto &f : a.preferredFacilitators){
            if(facilitator == f){
                score += 0.5;
                nope = true;
            }
        }
        //Check if facilitator was a listed one
        if(!nope){
            for(auto &f : a.otherFacilitators){
                if(facilitator == f){
                    score += 0.2;
                    nope = true;
                }
            }
        }
        //Otherwise facilitator is not good for the activity
        if(!nope)
            score -= 0.1;
    }


    return score;
}

//Room size fitness scoring
double calcRoomSizeScore(Schedule &s) {
    double score = 0.0;

    for (auto &item : s.schedule) {

        int capacity = item.room.capacity;
        int expected = item.activity.expEnrollment;

        if (capacity < expected) {
            score -= 0.5;
        } else if (capacity > expected * 6) {
            score -= 0.4;
        } else if (capacity > expected * 3) {
            score -= 0.2;
        } else {
            score += 0.3;
        }
    }

    return score;
}

double calcFacilitatorScore(Schedule &s){
    double score = 0.0;
    for(int i = 0; i < s.schedule.size(); i++){
        string f = s.schedule[i].facilitator;
        int occurrences = 1; //Tracks num of times same facilitator appears
        bool oneActivity = true;
        for(int j = i+1; j < s.schedule.size(); j++){
            //check for same facilitator at same time
            if(f == s.schedule[j].facilitator){
                occurrences++; //Increment when he/she appears again
                if(s.schedule[i].time == s.schedule[j].time)
                    oneActivity = false;
            }
        }
        //1 activity at this time slot or not?
        if(oneActivity)
            score += 0.2;
        else
            score -= 0.2;
        //More than 4 activities total?
        if(occurrences > 4)
            score -= 0.5;
        //1-2 activities total? Unless Dr. Tyler
        if(f != "Tyler"){
            if(occurrences == 1 || occurrences == 2 )
                score -= 0.4;
        }
    }
    return score;
}

unordered_map<string, int> timeIndex = {
    {"10 AM", 0},
    {"11 AM", 1},
    {"12 PM", 2},
    {"1 PM", 3},
    {"2 PM", 4},
    {"3 PM", 5}
};

//Activity specific conditions
double calcActivitySpecificScore(Schedule &s){
    double score = 0.0;
    ScheduleItem SLA101a = s.schedule[0];
    ScheduleItem SLA101b = s.schedule[1];
    ScheduleItem SLA191a = s.schedule[2];
    ScheduleItem SLA191b = s.schedule[3];
    int t0 = timeIndex[SLA101a.time];
    int t1 = timeIndex[SLA101b.time];
    int t2 = timeIndex[SLA191a.time];
    int t3 = timeIndex[SLA191b.time];
    //Are groups >4 hours apart or at same time?
    if(abs(t0 - t1) > 4)
        score+=0.5;
    else if(t0 == t1)
        score-=0.5;
    if(abs(t2 - t3) > 4)
        score+=0.5;
    else if(t2 == t3)
        score-=0.5;
    //Are cross groups 0-2 hours apart?
    if(abs(t0 - t2) == 1 || abs(t0 - t3) == 1 || abs(t1 - t2) == 1 || abs(t1 - t3) == 1)
        score+=0.5;
    else if(abs(t0 - t2) == 2 || abs(t0 - t3) == 2 || abs(t1 - t2) == 2 || abs(t1 - t3) == 2)
        score+=0.25;
    else if(abs(t0 - t2) == 0 || abs(t0 - t3) == 0 || abs(t1 - t2) == 0 || abs(t1 - t3) == 0)
        score-=0.25;

    return score;
}

double fitnessCheck(Schedule &s){
    double fitness = 0.0;
    //Checking activities
    fitness+=calcActivityScore(s);
    //Room size scoring
    fitness+=calcRoomSizeScore(s);
    //Facilitator scoring
    fitness+=calcFacilitatorScore(s);
    //Activity specific
    fitness+=calcActivitySpecificScore(s);

    return fitness;
}
//Compute score for all population
vector<double> computeFitnessScores(vector<Schedule> &population) {
    vector<double> fitnessScores;
    for (auto &schedule : population) {
        double fitness = fitnessCheck(schedule);
        fitnessScores.push_back(fitness);
    }
    return fitnessScores;
}

//Softmax function
vector<double> softmax(const vector<double> &scores) {
    vector<double> result;
    double maxScore = *max_element(scores.begin(), scores.end());

    double sum = 0.0;
    for (double score : scores) {
        sum += exp(score - maxScore);  // stability trick
    }

    for (double score : scores) {
        result.push_back(exp(score - maxScore) / sum);
    }

    return result;
}

//Parent selection
Schedule selectParent(const vector<Schedule> &population, const vector<double> &probabilities) {
    uniform_real_distribution<> dist(0.0, 1.0);
    double r = dist(rng);

    double cumulative = 0.0;
    for (size_t i = 0; i < probabilities.size(); ++i) {
        cumulative += probabilities[i];
        if (r <= cumulative) {
            return population[i]; // return a copy
        }
    }
    return population.back(); // fallback
}

//Create child
Schedule crossover(const Schedule &p1, const Schedule &p2) {
    Schedule child = p1;
    uniform_int_distribution<> dist(0, p1.schedule.size() - 1);
    int crossoverPoint = dist(rng);

    for (size_t i = crossoverPoint; i < p1.schedule.size(); ++i) {
        child.schedule[i] = p2.schedule[i];
    }

    return child;
}

//Mutation function
void mutate(Schedule &s, double mutationRate) {
    uniform_real_distribution<> probDist(0.0, 1.0);

    for (auto &item : s.schedule) {
        if (probDist(rng) < mutationRate) {
            item.time = getRandomElement(times);
        }
        if (probDist(rng) < mutationRate) {
            item.room = getRandomElement(rooms);
        }
        if (probDist(rng) < mutationRate) {
            item.facilitator = getRandomElement(facilitators);
        }
    }
}

vector<Schedule> evolvePopulation(const vector<Schedule> &population, double mutationRate) {
    vector<double> fitnessScores = computeFitnessScores(const_cast<vector<Schedule>&>(population));
    vector<double> probabilities = softmax(fitnessScores);

    vector<Schedule> newPopulation;

    while (newPopulation.size() < population.size()) {
        Schedule p1 = selectParent(population, probabilities);
        Schedule p2 = selectParent(population, probabilities);

        Schedule child = crossover(p1, p2);
        mutate(child, mutationRate);
        newPopulation.push_back(child);
    }

    return newPopulation;
}

//Create a population
vector<Schedule> generateInitialPopulation(int populationSize) {
    vector<Schedule> population;
    for (int i = 0; i < populationSize; ++i) {
        Schedule s = generateRandomSchedule();
        population.push_back(s);
    }
    return population;
}

//Run it all, and write to output file
void runEvolution(int generations, int popSize) {
    vector<Schedule> population = generateInitialPopulation(popSize);
    double mutationRate = 0.01;
    double prevAvgFitness = 0.0;

    for (int gen = 1; gen <= generations; ++gen) {
        vector<double> fitnessScores = computeFitnessScores(population);
        double avgFitness = accumulate(fitnessScores.begin(), fitnessScores.end(), 0.0) / fitnessScores.size();

        cout << "Generation " << gen << " | Avg Fitness: " << avgFitness << endl;

        if (gen == 100) prevAvgFitness = avgFitness;
        if (gen > 100 && avgFitness - prevAvgFitness < prevAvgFitness * 0.01) {
            cout << "Improvement < 1% over Gen100, stopping..." << endl;
            break;
        }

        population = evolvePopulation(population, mutationRate);

        // Optional: Halve mutation rate to fine-tune
        if (gen % 25 == 0 && mutationRate > 0.0001) {
            mutationRate /= 2;
        }
    }

    // Get best schedule
    Schedule best = *max_element(population.begin(), population.end(), [](Schedule &a, Schedule &b) {
        return fitnessCheck(a) < fitnessCheck(b);
    });

    // Save to file
    ofstream out("best_schedule.txt");
    for (auto &item : best.schedule) {
        out << item.activity.name << ", " << item.room.name << ", " << item.facilitator << ", " << item.time << "\n";
    }
    out << "Fitness Score: " << fitnessCheck(best);
    out.close();
}

int main(){

    int generations = 200;
    int populationSize = 50;

    runEvolution(generations, populationSize);
    
    return 0;
}