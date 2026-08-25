#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <cctype>
#include <mutex>
#include <algorithm>
#include "httplib.h"

using namespace std;

// Mutex for thread-safe file operations
mutex csvMutex;

// ============================================================
// READ A FILE
// ============================================================
string readRawFile(const string& path)
{
    ifstream file(path);
    if (!file.is_open())
    {
        cerr << "Failed to open file: " << path << endl;
        return "";
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// ============================================================
// TRIM WHITESPACE
// ============================================================
string trim(const string& str)
{
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == string::npos)
        return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

// ============================================================
// ROBUST CSV LINE PARSER (Handles quotes and commas correctly)
// ============================================================
vector<string> parseCsvLine(const string& line)
{
    vector<string> result;
    string current = "";
    bool inQuotes = false;
    for (size_t i = 0; i < line.size(); ++i)
    {
        char c = line[i];
        if (c == '"')
        {
            inQuotes = !inQuotes;
        }
        else if (c == ',' && !inQuotes)
        {
            result.push_back(current);
            current = "";
        }
        else
        {
            current += c;
        }
    }
    result.push_back(current);
    return result;
}

// ============================================================
// EXTRACT FIELD VALUE FROM JSON PAYLOAD
// ============================================================
string getJsonField(const string& json, const string& fieldName)
{
    const string key = "\"" + fieldName + "\"";
    size_t keyPos = json.find(key);
    if (keyPos == string::npos)
        return "";
    size_t colonPos = json.find(":", keyPos);
    if (colonPos == string::npos)
        return "";
    size_t valStart = colonPos + 1;
    while (valStart < json.size() && isspace(static_cast<unsigned char>(json[valStart])))
    {
        valStart++;
    }
    if (valStart >= json.size())
        return "";

    if (json[valStart] == '"')
    {
        // String value
        valStart++;
        size_t valEnd = json.find('"', valStart);
        if (valEnd == string::npos)
            return "";
        return json.substr(valStart, valEnd - valStart);
    }
    else
    {
        // Number or boolean value
        size_t valEnd = valStart;
        while (valEnd < json.size() && json[valEnd] != ',' && json[valEnd] != '}' && !isspace(static_cast<unsigned char>(json[valEnd])))
        {
            valEnd++;
        }
        return json.substr(valStart, valEnd - valStart);
    }
}

// Helper to unquote string values parsed from CSV
string unquoteField(string s)
{
    if (!s.empty() && s.front() == '"' && s.back() == '"')
        return s.substr(1, s.length() - 2);
    return s;
}

// Helper to calculate avatar initials from full name
string computeInitials(const string& name)
{
    string initials = "";
    if (!name.empty())
    {
        initials += static_cast<char>(toupper(name[0]));
        size_t spacePos = name.find(' ');
        if (spacePos != string::npos && spacePos + 1 < name.size())
        {
            initials += static_cast<char>(toupper(name[spacePos + 1]));
        }
    }
    return initials.empty() ? "U" : initials;
}

// ============================================================
// MAIN
// ============================================================
int main()
{
    httplib::Server svr;

    // ========================================================
    // SERVE PUBLIC FOLDER & DATA
    // ========================================================
    svr.set_mount_point("/", "./public");
    svr.set_mount_point("/data", "./data");

    // ========================================================
    // WEB PAGE ROUTES
    // ========================================================
    svr.Get("/", [](const httplib::Request& req, httplib::Response& res)
    {
        string html = readRawFile("public/index.html");
        if (!html.empty()) res.set_content(html, "text/html");
        else { res.status = 404; res.set_content("<h1>404 Not Found</h1>", "text/html"); }
    });
    svr.Get("/product", [](const httplib::Request& req, httplib::Response& res)
    {
        string html = readRawFile("public/product.html");
        if (!html.empty()) res.set_content(html, "text/html");
        else { res.status = 404; res.set_content("<h1>404 Not Found</h1>", "text/html"); }
    });
    svr.Get("/account", [](const httplib::Request& req, httplib::Response& res)
    {
        string html = readRawFile("public/account.html");
        if (!html.empty()) res.set_content(html, "text/html");
        else { res.status = 404; res.set_content("<h1>404 Not Found</h1>", "text/html"); }
    });
    svr.Get("/cart", [](const httplib::Request& req, httplib::Response& res)
    {
        string html = readRawFile("public/cart.html");
        if (!html.empty()) res.set_content(html, "text/html");
        else { res.status = 404; res.set_content("<h1>404 Not Found</h1>", "text/html"); }
    });
    svr.Get("/checkout", [](const httplib::Request& req, httplib::Response& res)
    {
        string html = readRawFile("public/checkout.html");
        if (!html.empty()) res.set_content(html, "text/html");
        else { res.status = 404; res.set_content("<h1>404 Not Found</h1>", "text/html"); }
    });
    svr.Get("/about", [](const httplib::Request& req, httplib::Response& res)
    {
        string html = readRawFile("public/about.html");
        if (!html.empty()) res.set_content(html, "text/html");
        else { res.status = 404; res.set_content("<h1>404 Not Found</h1>", "text/html"); }
    });
    svr.Get("/settings", [](const httplib::Request& req, httplib::Response& res)
    {
        string html = readRawFile("public/settings.html");
        if (!html.empty()) res.set_content(html, "text/html");
        else { res.status = 404; res.set_content("<h1>404 Not Found</h1>", "text/html"); }
    });
    svr.Get("/support", [](const httplib::Request& req, httplib::Response& res)
    {
        string html = readRawFile("public/support.html");
        if (!html.empty()) res.set_content(html, "text/html");
        else { res.status = 404; res.set_content("<h1>404 Not Found</h1>", "text/html"); }
    });

    // ========================================================
    // COMPONENT ROUTES
    // ========================================================
    svr.Get("/cart-drawer.html", [](const httplib::Request& req, httplib::Response& res)
    {
        string html = readRawFile("public/cart-drawer.html");
        if (!html.empty()) res.set_content(html, "text/html");
        else res.status = 404;
    });
    svr.Get("/review-modal.html", [](const httplib::Request& req, httplib::Response& res)
    {
        string html = readRawFile("public/review-modal.html");
        if (!html.empty()) res.set_content(html, "text/html");
        else res.status = 404;
    });
    svr.Get("/profile-modal.html", [](const httplib::Request& req, httplib::Response& res)
    {
        string html = readRawFile("public/profile-modal.html");
        if (!html.empty()) res.set_content(html, "text/html");
        else res.status = 404;
    });

    // ========================================================
    // PRODUCTS API
    // ========================================================
    svr.Get("/api/products", [](const httplib::Request& req, httplib::Response& res)
    {
        string json = R"([
            {
                "id": 1,
                "name": "Ethiopian Yirgacheffe G1",
                "tag": "SINGLE ORIGIN",
                "rating": 4.9,
                "reviewCount": 128,
                "price": 1250,
                "image": "https://images.unsplash.com/photo-1559056199-641a0ac8b55e?auto=format&fit=crop&w=600&q=80"
            },
            {
                "id": 2,
                "name": "Guatemala Antigua Medium",
                "tag": "BEST SELLER",
                "rating": 4.8,
                "reviewCount": 94,
                "price": 1100,
                "image": "https://images.unsplash.com/photo-1587734195503-904fca47e0e9?auto=format&fit=crop&w=600&q=80"
            },
            {
                "id": 3,
                "name": "Sumatra Mandheling Dark",
                "tag": "DARK ROAST",
                "rating": 4.7,
                "reviewCount": 82,
                "price": 1350,
                "image": "https://images.unsplash.com/photo-1611854779393-1b2da9d400fe?auto=format&fit=crop&w=600&q=80"
            }
        ])";
        res.set_content(json, "application/json");
    });

    // ========================================================
    // POST REGISTER ACCOUNT API (WITH ADDRESS)
    // ========================================================
    svr.Post("/api/register", [](const httplib::Request& req, httplib::Response& res)
    {
        const string usersFilePath = "data/users.csv";

        string name = getJsonField(req.body, "name");
        string email = getJsonField(req.body, "email");
        string phone = getJsonField(req.body, "phone");
        string address = getJsonField(req.body, "address");
        string password = getJsonField(req.body, "password");

        if (name.empty() || email.empty() || password.empty())
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Missing required fields"})", "application/json");
            return;
        }

        lock_guard<mutex> lock(csvMutex);
        
        ifstream checkFile(usersFilePath);
        bool isEmpty = !checkFile.is_open() || checkFile.peek() == ifstream::traits_type::eof();
        checkFile.close();

        ofstream outFile(usersFilePath, ios::out | ios::app);
        if (!outFile.is_open())
        {
            res.status = 500;
            res.set_content(R"({"status":"error","message":"Failed to open users.csv"})", "application/json");
            return;
        }

        if (isEmpty)
        {
            outFile << "id,name,email,phone,address,password,avatarInitials,membershipTier,points,pointsValue,subscription\n";
        }

        string accountId = "ACC" + to_string(time(nullptr));
        string initials = computeInitials(name);

        outFile << "\"" << accountId << "\","
                << "\"" << name << "\","
                << "\"" << email << "\","
                << "\"" << phone << "\","
                << "\"" << address << "\","
                << "\"" << password << "\","
                << "\"" << initials << "\","
                << "\"Member\","
                << "50,"
                << "\"NRs. 50\","
                << "\"No active subscription\"\n";

        outFile.close();

        res.status = 200;
        res.set_content(R"({"status":"success","message":"Account created successfully"})", "application/json");
    });

    // ========================================================
    // POST UPDATE USER PROFILE API (BY EMAIL)
    // ========================================================
    svr.Post("/api/user/update", [](const httplib::Request& req, httplib::Response& res)
    {
        const string usersFilePath = "data/users.csv";

        string email = getJsonField(req.body, "email");
        string name = getJsonField(req.body, "name");
        string phone = getJsonField(req.body, "phone");
        string address = getJsonField(req.body, "address");

        if (email.empty())
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Email is required"})", "application/json");
            return;
        }

        lock_guard<mutex> lock(csvMutex);
        ifstream inFile(usersFilePath);
        if (!inFile.is_open())
        {
            res.status = 500;
            res.set_content(R"({"status":"error","message":"Cannot open users.csv"})", "application/json");
            return;
        }

        vector<string> updatedLines;
        string line;
        string headerLine = "";
        bool headerRead = false;
        bool found = false;

        while (getline(inFile, line))
        {
            if (line.empty()) continue;
            if (!headerRead)
            {
                headerLine = line;
                headerRead = true;
                continue;
            }

            vector<string> fields = parseCsvLine(line);
            if (fields.size() < 11)
            {
                updatedLines.push_back(line);
                continue;
            }

            string rowEmail = unquoteField(fields[2]);
            if (rowEmail == email)
            {
                found = true;
                fields[1] = "\"" + name + "\"";
                fields[3] = "\"" + phone + "\"";
                fields[4] = "\"" + address + "\"";
                fields[6] = "\"" + computeInitials(name) + "\"";

                stringstream newLine;
                for (size_t i = 0; i < fields.size(); ++i)
                {
                    newLine << fields[i];
                    if (i + 1 < fields.size()) newLine << ",";
                }
                updatedLines.push_back(newLine.str());
            }
            else
            {
                updatedLines.push_back(line);
            }
        }
        inFile.close();

        if (!found)
        {
            res.status = 404;
            res.set_content(R"({"status":"error","message":"User not found"})", "application/json");
            return;
        }

        ofstream outFile(usersFilePath, ios::out | ios::trunc);
        outFile << (headerLine.empty() ? "id,name,email,phone,address,password,avatarInitials,membershipTier,points,pointsValue,subscription" : headerLine) << "\n";
        for (const auto& l : updatedLines)
        {
            outFile << l << "\n";
        }
        outFile.close();

        res.status = 200;
        res.set_content(R"({"status":"success","message":"Profile updated successfully"})", "application/json");
    });

    // ========================================================
    // POST ADD TO CART
    // ========================================================
    svr.Post("/api/cart/add", [](const httplib::Request& req, httplib::Response& res)
    {
        const string filepath = "data/cart.csv";
        if (req.body.empty())
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Empty request body"})", "application/json");
            return;
        }

        string accountId = getJsonField(req.body, "account_id");
        string email = getJsonField(req.body, "email");
        string productId = getJsonField(req.body, "product_id");
        string productName = getJsonField(req.body, "product_name");
        string qtyStr = getJsonField(req.body, "quantity");
        string price = getJsonField(req.body, "price");
        string addedFrom = getJsonField(req.body, "added_from");

        if (accountId.empty() || productId.empty())
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Missing required fields"})", "application/json");
            return;
        }

        int addQty = qtyStr.empty() ? 1 : stoi(qtyStr);

        lock_guard<mutex> lock(csvMutex);

        vector<vector<string>> rows;
        bool headerExists = false;
        
        ifstream inFile(filepath);
        if (inFile.is_open())
        {
            string line;
            bool isFirst = true;
            while (getline(inFile, line))
            {
                if (line.empty()) continue;
                if (isFirst)
                {
                    headerExists = true;
                    isFirst = false;
                    rows.push_back(parseCsvLine(line));
                    continue;
                }
                rows.push_back(parseCsvLine(line));
            }
            inFile.close();
        }

        time_t now = time(0);
        char timestamp[80];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

        bool found = false;
        for (size_t i = 1; i < rows.size(); ++i)
        {
            if (rows[i].size() >= 6 && rows[i][0] == accountId && rows[i][2] == productId && rows[i][5] == price)
            {
                int currentQty = stoi(rows[i][4]);
                rows[i][4] = to_string(currentQty + addQty);
                if (rows[i].size() > 7) {
                    rows[i][7] = timestamp;
                } else {
                    rows[i].push_back(timestamp);
                }
                found = true;
                break;
            }
        }

        if (!found)
        {
            if (!headerExists && rows.empty())
            {
                rows.push_back({"account_id", "email", "product_id", "product_name", "quantity", "price", "added_from", "timestamp"});
            }
            rows.push_back({accountId, email, productId, productName, to_string(addQty), price, addedFrom, timestamp});
        }

        ofstream outFile(filepath, ios::out | ios::trunc);
        if (!outFile.is_open())
        {
            res.status = 500;
            res.set_content(R"({"status":"error","message":"Cannot open cart.csv for writing"})", "application/json");
            return;
        }

        for (const auto& row : rows)
        {
            for (size_t j = 0; j < row.size(); ++j)
            {
                outFile << "\"" << row[j] << "\"";
                if (j + 1 < row.size()) outFile << ",";
            }
            outFile << "\n";
        }
        outFile.close();

        res.status = 200;
        res.set_content(R"({"status":"success","message":"Cart updated successfully"})", "application/json");
    });
    
    // ========================================================
    // GET ALL USERS API (WITH ADDRESS FIELD)
    // ========================================================
    svr.Get("/api/users", [](const httplib::Request& req, httplib::Response& res)
    {
        const string filepath = "data/users.csv";
        lock_guard<mutex> lock(csvMutex);
        ifstream file(filepath);
        if (!file.is_open())
        {
            res.set_content("[]", "application/json");
            return;
        }

        string line;
        getline(file, line); // Skip header

        string jsonArray = "[";
        bool first = true;

        while (getline(file, line))
        {
            if (line.empty()) continue;
            vector<string> fields = parseCsvLine(line);
            if (fields.size() < 11) continue;

            string id = unquoteField(fields[0]);
            string name = unquoteField(fields[1]);
            string email = unquoteField(fields[2]);
            string phone = unquoteField(fields[3]);
            string address = unquoteField(fields[4]);
            string password = unquoteField(fields[5]);
            string initials = unquoteField(fields[6]);
            string tier = unquoteField(fields[7]);
            string points = unquoteField(fields[8]);
            string pointsVal = unquoteField(fields[9]);
            string sub = unquoteField(fields[10]);

            if (!first) jsonArray += ",";
            first = false;

            jsonArray += "{";
            jsonArray += "\"id\":\"" + id + "\",";
            jsonArray += "\"name\":\"" + name + "\",";
            jsonArray += "\"email\":\"" + email + "\",";
            jsonArray += "\"phone\":\"" + phone + "\",";
            jsonArray += "\"address\":\"" + address + "\",";
            jsonArray += "\"password\":\"" + password + "\",";
            jsonArray += "\"avatarInitials\":\"" + initials + "\",";
            jsonArray += "\"membershipTier\":\"" + tier + "\",";
            jsonArray += "\"points\":" + (points.empty() ? "50" : points) + ",";
            jsonArray += "\"pointsValue\":\"" + pointsVal + "\",";
            jsonArray += "\"subscription\":\"" + sub + "\"";
            jsonArray += "}";
        }
        jsonArray += "]";
        res.set_content(jsonArray, "application/json");
    });

    // ========================================================
    // GET REVIEWS (CSV)
    // ========================================================
    svr.Get("/api/reviews", [](const httplib::Request& req, httplib::Response& res)
    {
        const string filepath = "data/reviews.csv";
        lock_guard<mutex> lock(csvMutex);
        string content = readRawFile(filepath);

        if (content.empty())
        {
            content = "productId,rating,title,text,recommend,name,email,verified,date\n";
        }

        res.set_content(content, "text/csv");
    });

    // ========================================================
    // POST REVIEW (CSV APPEND)
    // ========================================================
    svr.Post("/api/reviews", [](const httplib::Request& req, httplib::Response& res)
    {
        const string filepath = "data/reviews.csv";
        if (req.body.empty())
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Empty request body"})", "application/json");
            return;
        }
        string productId = getJsonField(req.body, "productId");
        string rating = getJsonField(req.body, "rating");
        string title = getJsonField(req.body, "title");
        string text = getJsonField(req.body, "text");
        string recommend = getJsonField(req.body, "recommend");
        string name = getJsonField(req.body, "name");
        string email = getJsonField(req.body, "email");
        string verified = getJsonField(req.body, "verified");
        string date = getJsonField(req.body, "date");
        if (productId.empty())
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Invalid productId"})", "application/json");
            return;
        }
        stringstream csvLine;
        csvLine << productId << ","
                << (rating.empty() ? "5" : rating) << ","
                << "\"" << title << "\","
                << "\"" << text << "\","
                << (recommend.empty() ? "yes" : recommend) << ","
                << "\"" << name << "\","
                << (email.empty() ? "example@email.com" : email) << ","
                << (verified.empty() ? "True" : verified) << ","
                << "\"" << date << "\"\n";
        
        lock_guard<mutex> lock(csvMutex);
        ofstream outFile(filepath, ios::out | ios::app);
        if (!outFile.is_open())
        {
            res.status = 500;
            res.set_content(R"({"status":"error","message":"Cannot open reviews.csv for writing"})", "application/json");
            return;
        }
        outFile << csvLine.str();
        outFile.close();
        res.status = 200;
        res.set_content(R"({"status":"success","message":"Review saved successfully to CSV"})", "application/json");
    });

    // ========================================================
    // POST USER REGISTRATION (ALTERNATIVE ADMIN / SYSTEM ROUTE)
    // ========================================================
    svr.Post("/api/users", [](const httplib::Request& req, httplib::Response& res)
    {
        const string filepath = "data/users.csv";
        if (req.body.empty())
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Empty request body"})", "application/json");
            return;
        }

        string id = getJsonField(req.body, "id");
        string name = getJsonField(req.body, "name");
        string email = getJsonField(req.body, "email");
        string phone = getJsonField(req.body, "phone");
        string address = getJsonField(req.body, "address");
        string password = getJsonField(req.body, "password");
        string avatarInitials = getJsonField(req.body, "avatarInitials");
        string membershipTier = getJsonField(req.body, "membershipTier");
        string points = getJsonField(req.body, "points");
        string pointsValue = getJsonField(req.body, "pointsValue");
        string subscription = getJsonField(req.body, "subscription");

        if (email.empty() || password.empty())
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Invalid email or password"})", "application/json");
            return;
        }

        lock_guard<mutex> lock(csvMutex);
        ifstream checkFile(filepath);
        bool isEmpty = !checkFile.is_open() || checkFile.peek() == ifstream::traits_type::eof();
        checkFile.close();

        ofstream outFile(filepath, ios::out | ios::app);
        if (!outFile.is_open())
        {
            res.status = 500;
            res.set_content(R"({"status":"error","message":"Cannot open users.csv for writing"})", "application/json");
            return;
        }

        if (isEmpty)
        {
            outFile << "id,name,email,phone,address,password,avatarInitials,membershipTier,points,pointsValue,subscription\n";
        }

        if (avatarInitials.empty())
        {
            avatarInitials = computeInitials(name);
        }

        stringstream csvLine;
        csvLine << "\"" << (id.empty() ? "ACC" + to_string(time(nullptr)) : id) << "\","
                << "\"" << name << "\","
                << "\"" << email << "\","
                << "\"" << phone << "\","
                << "\"" << address << "\","
                << "\"" << password << "\","
                << "\"" << avatarInitials << "\","
                << "\"" << (membershipTier.empty() ? "Member" : membershipTier) << "\","
                << (points.empty() ? "50" : points) << ","
                << "\"" << (pointsValue.empty() ? "NRs. 50" : pointsValue) << "\","
                << "\"" << (subscription.empty() ? "No active subscription" : subscription) << "\"\n";

        outFile << csvLine.str();
        outFile.close();

        res.status = 200;
        res.set_content(R"({"status":"success","message":"User saved successfully"})", "application/json");
    });

    // ========================================================
    // POST USER LOGIN / AUTHENTICATION API (RETURNS ADDRESS)
    // ========================================================
    svr.Post("/api/login", [](const httplib::Request& req, httplib::Response& res)
    {
        const string filepath = "data/users.csv";
        string emailInput = getJsonField(req.body, "email");
        string passInput = getJsonField(req.body, "password");

        lock_guard<mutex> lock(csvMutex);
        ifstream file(filepath);
        if (!file.is_open())
        {
            res.status = 500;
            res.set_content(R"({"status":"error","message":"Database error"})", "application/json");
            return;
        }

        string line;
        getline(file, line); // Skip header

        bool authenticated = false;
        string foundId = "1", foundName = "", foundEmail = "", foundPhone = "", foundAddress = "", foundPass = "", foundInitials = "U", foundTier = "", foundPoints = "50", foundVal = "NRs. 50", foundSub = "";

        while (getline(file, line))
        {
            if (line.empty()) continue;
            vector<string> fields = parseCsvLine(line);
            if (fields.size() < 11) continue;

            string id = fields[0];
            string name = fields[1];
            string email = fields[2];
            string phone = fields[3];
            string address = fields[4];
            string password = fields[5];
            string initials = fields[6];
            string tier = fields[7];
            string points = fields[8];
            string pointsVal = fields[9];
            string sub = fields[10];

            string cleanEmail = unquoteField(email);
            string cleanPass = unquoteField(password);

            if (cleanEmail == emailInput && cleanPass == passInput)
            {
                authenticated = true;
                foundId = id.empty() ? "1" : unquoteField(id);
                foundName = unquoteField(name);
                foundEmail = cleanEmail;
                foundPhone = unquoteField(phone);
                foundAddress = unquoteField(address);
                foundPass = cleanPass;
                foundInitials = unquoteField(initials);
                foundTier = unquoteField(tier);
                foundPoints = points.empty() ? "50" : unquoteField(points);
                foundVal = unquoteField(pointsVal);
                foundSub = unquoteField(sub);
                break;
            }
        }
        file.close();

        if (authenticated)
        {
            string jsonResp = "{";
            jsonResp += "\"status\":\"success\",";
            jsonResp += "\"message\":\"Authenticated\",";
            jsonResp += "\"user\":{";
            jsonResp += "\"id\":\"" + foundId + "\",";
            jsonResp += "\"name\":\"" + foundName + "\",";
            jsonResp += "\"email\":\"" + foundEmail + "\",";
            jsonResp += "\"phone\":\"" + foundPhone + "\",";
            jsonResp += "\"address\":\"" + foundAddress + "\",";
            jsonResp += "\"password\":\"" + foundPass + "\",";
            jsonResp += "\"avatarInitials\":\"" + foundInitials + "\",";
            jsonResp += "\"membershipTier\":\"" + foundTier + "\",";
            jsonResp += "\"points\":" + foundPoints + ",";
            jsonResp += "\"pointsValue\":\"" + foundVal + "\",";
            jsonResp += "\"subscription\":\"" + foundSub + "\"";
            jsonResp += "}}";

            res.status = 200;
            res.set_content(jsonResp, "application/json");
        }
        else
        {
            res.status = 401;
            res.set_content(R"({"status":"error","message":"Invalid email or password"})", "application/json");
        }
    });

    // ========================================================
    // POST UPDATE CART ITEM QUANTITY
    // ========================================================
    svr.Post("/api/cart/update", [](const httplib::Request& req, httplib::Response& res)
    {
        const string filepath = "data/cart.csv";
        if (req.body.empty())
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Empty request body"})", "application/json");
            return;
        }

        string targetEmail = getJsonField(req.body, "email");
        string targetProductId = getJsonField(req.body, "product_id");
        string deltaStr = getJsonField(req.body, "delta");
        int delta = deltaStr.empty() ? 1 : stoi(deltaStr);

        if (targetEmail.empty() || targetProductId.empty())
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Missing required fields"})", "application/json");
            return;
        }

        lock_guard<mutex> lock(csvMutex);
        ifstream file(filepath);
        vector<string> lines;
        string headerLine = "";
        bool headerRead = false;
        string line;

        while (getline(file, line))
        {
            if (!headerRead)
            {
                headerLine = line;
                headerRead = true;
                continue;
            }
            if (line.empty()) continue;
            lines.push_back(line);
        }
        file.close();

        vector<vector<string>> parsedRows;
        for (const auto& l : lines)
        {
            parsedRows.push_back(parseCsvLine(l));
        }

        ofstream outFile(filepath, ios::out | ios::trunc);
        if (!outFile.is_open())
        {
            res.status = 500;
            res.set_content(R"({"status":"error","message":"Cannot open cart.csv for writing"})", "application/json");
            return;
        }

        outFile << (headerLine.empty() ? "account_id,email,product_id,product_name,quantity,price,added_from,timestamp" : headerLine) << "\n";

        for (auto& fields : parsedRows)
        {
            if (fields.size() < 8) continue;
            string account_id = fields[0];
            string email = fields[1];
            string product_id = fields[2];
            string product_name = fields[3];
            string quantity_str = fields[4];
            string price = fields[5];
            string added_from = fields[6];
            string timestamp = fields[7];

            if (unquoteField(email) == targetEmail && unquoteField(product_id) == targetProductId)
            {
                int currentQty = stoi(quantity_str.empty() ? "1" : unquoteField(quantity_str));
                currentQty += delta;
                if (currentQty <= 0)
                {
                    continue; 
                }
                quantity_str = to_string(currentQty);
            }

            outFile << "\"" << unquoteField(account_id) << "\","
                    << "\"" << unquoteField(email) << "\","
                    << "\"" << unquoteField(product_id) << "\","
                    << "\"" << unquoteField(product_name) << "\","
                    << "\"" << unquoteField(quantity_str) << "\","
                    << "\"" << unquoteField(price) << "\","
                    << "\"" << unquoteField(added_from) << "\","
                    << "\"" << unquoteField(timestamp) << "\"\n";
        }
        outFile.close();

        res.status = 200;
        res.set_content(R"({"status":"success","message":"Cart updated successfully"})", "application/json");
    });

    // ========================================================
    // POST REMOVE CART ITEM
    // ========================================================
    svr.Post("/api/cart/remove", [](const httplib::Request& req, httplib::Response& res)
    {
        const string filepath = "data/cart.csv";
        if (req.body.empty())
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Empty request body"})", "application/json");
            return;
        }

        string targetEmail = getJsonField(req.body, "email");
        string targetProductId = getJsonField(req.body, "product_id");

        if (targetEmail.empty() || targetProductId.empty())
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Missing required fields"})", "application/json");
            return;
        }

        lock_guard<mutex> lock(csvMutex);
        ifstream file(filepath);
        vector<string> lines;
        string headerLine = "";
        bool headerRead = false;
        string line;

        while (getline(file, line))
        {
            if (!headerRead)
            {
                headerLine = line;
                headerRead = true;
                continue;
            }
            if (line.empty()) continue;
            lines.push_back(line);
        }
        file.close();

        vector<vector<string>> parsedRows;
        for (const auto& l : lines)
        {
            parsedRows.push_back(parseCsvLine(l));
        }

        ofstream outFile(filepath, ios::out | ios::trunc);
        if (!outFile.is_open())
        {
            res.status = 500;
            res.set_content(R"({"status":"error","message":"Cannot open cart.csv for writing"})", "application/json");
            return;
        }

        outFile << (headerLine.empty() ? "account_id,email,product_id,product_name,quantity,price,added_from,timestamp" : headerLine) << "\n";

        for (auto& fields : parsedRows)
        {
            if (fields.size() < 8) continue;
            string account_id = fields[0];
            string email = fields[1];
            string product_id = fields[2];
            string product_name = fields[3];
            string quantity_str = fields[4];
            string price = fields[5];
            string added_from = fields[6];
            string timestamp = fields[7];

            if (unquoteField(email) == targetEmail && unquoteField(product_id) == targetProductId)
            {
                continue;
            }

            outFile << "\"" << unquoteField(account_id) << "\","
                    << "\"" << unquoteField(email) << "\","
                    << "\"" << unquoteField(product_id) << "\","
                    << "\"" << unquoteField(product_name) << "\","
                    << "\"" << unquoteField(quantity_str) << "\","
                    << "\"" << unquoteField(price) << "\","
                    << "\"" << unquoteField(added_from) << "\","
                    << "\"" << unquoteField(timestamp) << "\"\n";
        }
        outFile.close();

        res.status = 200;
        res.set_content(R"({"status":"success","message":"Item removed successfully"})", "application/json");
    });

    // ========================================================
    // CHECK WISHLIST API
    // ========================================================
    svr.Get("/api/wishlist/check", [](const httplib::Request& req, httplib::Response& res)
    {
        string accountId = req.get_param_value("account_id");
        string productId = req.get_param_value("product_id");

        const string filepath = "data/wishlist.csv";
        lock_guard<mutex> lock(csvMutex);
        ifstream file(filepath);
        if (!file.is_open())
        {
            res.set_content(R"({"isWishlisted":false})", "application/json");
            return;
        }

        string line;
        bool found = false;
        while (getline(file, line))
        {
            if (line.empty()) continue;
            vector<string> fields = parseCsvLine(line);
            if (fields.size() >= 3 && unquoteField(fields[0]) == accountId && unquoteField(fields[2]) == productId)
            {
                found = true;
                break;
            }
        }
        file.close();

        if (found)
            res.set_content(R"({"isWishlisted":true})", "application/json");
        else
            res.set_content(R"({"isWishlisted":false})", "application/json");
    });

    // ========================================================
    // GET USER WISHLIST API
    // ========================================================
    svr.Get("/api/wishlist", [](const httplib::Request& req, httplib::Response& res)
    {
        string accountId = req.get_param_value("account_id");
        const string filepath = "data/wishlist.csv";
        
        lock_guard<mutex> lock(csvMutex);
        ifstream file(filepath);
        if (!file.is_open())
        {
            res.set_content("[]", "application/json");
            return;
        }

        string line;
        getline(file, line); // Skip header

        string jsonArray = "[";
        bool first = true;

        while (getline(file, line))
        {
            if (line.empty()) continue;
            vector<string> fields = parseCsvLine(line);
            if (fields.size() < 6) continue;

            string rowAccId = unquoteField(fields[0]);
            if (rowAccId != accountId) continue;

            string email = unquoteField(fields[1]);
            string productId = unquoteField(fields[2]);
            string productName = unquoteField(fields[3]);
            string price = unquoteField(fields[4]);
            string timestamp = unquoteField(fields[5]);

            if (!first) jsonArray += ",";
            first = false;

            jsonArray += "{";
            jsonArray += "\"account_id\":\"" + rowAccId + "\",";
            jsonArray += "\"email\":\"" + email + "\",";
            jsonArray += "\"product_id\":\"" + productId + "\",";
            jsonArray += "\"product_name\":\"" + productName + "\",";
            jsonArray += "\"price\":\"" + price + "\",";
            jsonArray += "\"timestamp\":\"" + timestamp + "\"";
            jsonArray += "}";
        }
        jsonArray += "]";
        res.set_content(jsonArray, "application/json");
    });

    // ========================================================
    // POST ADD TO WISHLIST API
    // ========================================================
    svr.Post("/api/wishlist/add", [](const httplib::Request& req, httplib::Response& res)
    {
        const string filepath = "data/wishlist.csv";
        if (req.body.empty())
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Empty request body"})", "application/json");
            return;
        }

        string accountId = getJsonField(req.body, "account_id");
        string email = getJsonField(req.body, "email");
        string productId = getJsonField(req.body, "product_id");
        string productName = getJsonField(req.body, "product_name");
        string price = getJsonField(req.body, "price");

        if (accountId.empty() || productId.empty())
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Missing required fields"})", "application/json");
            return;
        }

        lock_guard<mutex> lock(csvMutex);
        vector<vector<string>> rows;
        bool headerExists = false;
        ifstream inFile(filepath);
        if (inFile.is_open())
        {
            string line;
            bool isFirst = true;
            while (getline(inFile, line))
            {
                if (line.empty()) continue;
                if (isFirst) { headerExists = true; isFirst = false; }
                rows.push_back(parseCsvLine(line));
            }
            inFile.close();
        }

        for (size_t i = 1; i < rows.size(); ++i)
        {
            if (rows[i].size() >= 3 && unquoteField(rows[i][0]) == accountId && unquoteField(rows[i][2]) == productId)
            {
                res.status = 200;
                res.set_content(R"({"status":"success","message":"Already in wishlist"})", "application/json");
                return;
            }
        }

        time_t now = time(0);
        char timestamp[80];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

        ofstream outFile(filepath, ios::out | ios::app);
        if (!outFile.is_open())
        {
            res.status = 500;
            res.set_content(R"({"status":"error","message":"Cannot open wishlist.csv"})", "application/json");
            return;
        }

        if (!headerExists)
        {
            outFile << "account_id,email,product_id,product_name,price,timestamp\n";
        }

        outFile << "\"" << accountId << "\","
                << "\"" << email << "\","
                << "\"" << productId << "\","
                << "\"" << productName << "\","
                << "\"" << price << "\","
                << "\"" << timestamp << "\"\n";
        outFile.close();

        res.status = 200;
        res.set_content(R"({"status":"success","message":"Added to wishlist successfully"})", "application/json");
    });

    // ========================================================
    // REMOVE FROM WISHLIST API
    // ========================================================
    svr.Post("/api/wishlist/remove", [](const httplib::Request& req, httplib::Response& res)
    {
        const string filepath = "data/wishlist.csv";
        if (req.body.empty())
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Empty request body"})", "application/json");
            return;
        }

        string accountId = getJsonField(req.body, "account_id");
        string productId = getJsonField(req.body, "product_id");

        if (accountId.empty() || productId.empty())
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Missing required fields"})", "application/json");
            return;
        }

        lock_guard<mutex> lock(csvMutex);
        vector<vector<string>> rows;
        string headerLine = "";
        bool headerRead = false;
        
        ifstream inFile(filepath);
        if (inFile.is_open())
        {
            string line;
            while (getline(inFile, line))
            {
                if (line.empty()) continue;
                if (!headerRead)
                {
                    headerLine = line;
                    headerRead = true;
                    continue;
                }
                
                vector<string> parsedRow = parseCsvLine(line);
                if (parsedRow.size() >= 3 && unquoteField(parsedRow[0]) == accountId && unquoteField(parsedRow[2]) == productId)
                {
                    continue;
                }
                rows.push_back(parsedRow);
            }
            inFile.close();
        }

        ofstream outFile(filepath, ios::trunc);
        if (outFile.is_open())
        {
            if (!headerLine.empty())
            {
                outFile << headerLine << "\n";
            }
            else
            {
                outFile << "account_id,email,product_id,product_name,price,timestamp\n";
            }

            for (const auto& row : rows)
            {
                for (size_t i = 0; i < row.size(); ++i)
                {
                    outFile << "\"" << unquoteField(row[i]) << "\"";
                    if (i + 1 < row.size()) outFile << ",";
                }
                outFile << "\n";
            }
            outFile.close();
        }

        res.set_content(R"({"status":"success","message":"Item removed from wishlist"})", "application/json");
    });

    // ========================================================
    // POST UPDATE USER BY ID API (INCLUDES ADDRESS)
    // ========================================================
    svr.Post("/api/update-user", [](const httplib::Request& req, httplib::Response& res)
    {
        const string filepath = "data/users.csv";
        if (req.body.empty())
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Empty request body"})", "application/json");
            return;
        }

        string targetId = getJsonField(req.body, "id");
        string newName = getJsonField(req.body, "name");
        string newEmail = getJsonField(req.body, "email");
        string newPhone = getJsonField(req.body, "phone");
        string newAddress = getJsonField(req.body, "address");
        string newPassword = getJsonField(req.body, "password");

        if (targetId.empty())
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Missing user ID"})", "application/json");
            return;
        }

        lock_guard<mutex> lock(csvMutex);
        
        vector<string> lines;
        string headerLine = "";
        bool headerRead = false;
        
        ifstream infile(filepath);
        if (infile.is_open())
        {
            string line;
            while (getline(infile, line))
            {
                if (line.empty()) continue;
                if (!headerRead)
                {
                    headerLine = line;
                    headerRead = true;
                    continue;
                }
                lines.push_back(line);
            }
            infile.close();
        }

        bool updated = false;
        vector<vector<string>> parsedRows;
        
        for (const auto& l : lines)
        {
            vector<string> fields = parseCsvLine(l);
            if (fields.size() < 11)
            {
                parsedRows.push_back(fields);
                continue;
            }

            string id = unquoteField(fields[0]);
            if (id == targetId)
            {
                fields[1] = "\"" + newName + "\"";
                fields[2] = "\"" + newEmail + "\"";
                fields[3] = "\"" + newPhone + "\"";
                fields[4] = "\"" + newAddress + "\"";
                fields[5] = "\"" + newPassword + "\"";
                fields[6] = "\"" + computeInitials(newName) + "\"";
                updated = true;
            }
            parsedRows.push_back(fields);
        }

        ofstream outfile(filepath, ios::out | ios::trunc);
        if (!outfile.is_open())
        {
            res.status = 500;
            res.set_content(R"({"status":"error","message":"Cannot open users.csv for writing"})", "application/json");
            return;
        }

        outfile << (headerLine.empty() ? "id,name,email,phone,address,password,avatarInitials,membershipTier,points,pointsValue,subscription" : headerLine) << "\n";

        for (const auto& row : parsedRows)
        {
            for (size_t i = 0; i < row.size(); ++i)
            {
                outfile << row[i];
                if (i + 1 < row.size()) outfile << ",";
            }
            outfile << "\n";
        }
        outfile.close();

        if (updated)
        {
            res.status = 200;
            res.set_content(R"({"status":"success","message":"Profile updated successfully"})", "application/json");
        }
        else
        {
            res.status = 404;
            res.set_content(R"({"status":"error","message":"User ID not found in CSV"})", "application/json");
        }
    });
    
    // ========================================================
    // GET CART COUNT API
    // ========================================================
    svr.Get("/api/cart-count", [](const httplib::Request& req, httplib::Response& res)
    {
        string userEmail = req.get_param_value("email");
        if (userEmail.empty())
        {
            res.status = 400;
            res.set_content(R"({"count": 0})", "application/json");
            return;
        }

        const string filepath = "data/cart.csv";
        lock_guard<mutex> lock(csvMutex);
        ifstream file(filepath);
        if (!file.is_open())
        {
            res.set_content(R"({"count": 0})", "application/json");
            return;
        }

        string line;
        getline(file, line); // Skip header row

        int count = 0;

        while (getline(file, line))
        {
            if (line.empty()) continue;
            vector<string> fields = parseCsvLine(line);
            if (fields.size() < 5) continue;

            string rowEmail = unquoteField(fields[1]);
            if (rowEmail == userEmail)
            {
                int qty = stoi(unquoteField(fields[4]).empty() ? "1" : unquoteField(fields[4]));
                count += qty;
            }
        }
        file.close();

        res.status = 200;
        res.set_content("{\"count\": " + to_string(count) + "}", "application/json");
    });

    // ========================================================
    // POST ORDERS API (SAVES ORDER & CLEARS USER CART)
    // ========================================================
    svr.Post("/api/orders", [](const httplib::Request& req, httplib::Response& res)
    {
        const string ordersFilePath = "data/orders.csv";
        const string cartFilePath = "data/cart.csv";

        if (req.body.empty())
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Empty request body"})", "application/json");
            return;
        }

        string fullName = getJsonField(req.body, "fullName");
        string phone = getJsonField(req.body, "phone");
        string address = getJsonField(req.body, "address");
        string city = getJsonField(req.body, "city");
        string notes = getJsonField(req.body, "notes");
        string paymentMethod = getJsonField(req.body, "paymentMethod");
        string date = getJsonField(req.body, "date");
        string email = getJsonField(req.body, "email");

        if (fullName.empty() || phone.empty() || address.empty())
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Missing required fields"})", "application/json");
            return;
        }

        // --- 1. WRITE TO ORDERS.CSV ---
        {
            lock_guard<mutex> lock(csvMutex);
            ifstream checkFile(ordersFilePath);
            bool isEmpty = !checkFile.is_open() || checkFile.peek() == ifstream::traits_type::eof();
            checkFile.close();

            ofstream outFile(ordersFilePath, ios::out | ios::app);
            if (!outFile.is_open())
            {
                res.status = 500;
                res.set_content(R"({"status":"error","message":"Cannot open orders.csv for writing"})", "application/json");
                return;
            }

            if (isEmpty)
            {
                outFile << "fullName,phone,address,city,notes,paymentMethod,date,email\n";
            }

            stringstream csvLine;
            csvLine << "\"" << fullName << "\","
                    << "\"" << phone << "\","
                    << "\"" << address << "\","
                    << "\"" << city << "\","
                    << "\"" << notes << "\","
                    << "\"" << paymentMethod << "\","
                    << "\"" << date << "\","
                    << "\"" << email << "\"\n";

            outFile << csvLine.str();
            outFile.close();
        }

        // --- 2. CLEAR USER'S CART FROM CART.CSV ---
        if (!email.empty())
        {
            lock_guard<mutex> lock(csvMutex);
            ifstream inFile(cartFilePath);
            if (inFile.is_open())
            {
                vector<string> remainingLines;
                string line;

                string targetEmail = email;
                transform(targetEmail.begin(), targetEmail.end(), targetEmail.begin(), ::tolower);

                while (getline(inFile, line))
                {
                    if (line.empty()) continue;

                    string lowerLine = line;
                    transform(lowerLine.begin(), lowerLine.end(), lowerLine.begin(), ::tolower);

                    if (lowerLine.find(targetEmail) == string::npos)
                    {
                        remainingLines.push_back(line);
                    }
                }
                inFile.close();

                ofstream outFile(cartFilePath, ios::out | ios::trunc);
                if (outFile.is_open())
                {
                    for (const auto& remainingLine : remainingLines)
                    {
                        outFile << remainingLine << "\n";
                    }
                    outFile.close();
                }
            }
        }

        res.status = 200;
        res.set_content(R"({"status":"success","message":"Order placed and cart cleared successfully"})", "application/json");
    });
    
    // ========================================================
// GET ORDERS HISTORY API BY EMAIL
// ========================================================
svr.Get("/api/orders", [](const httplib::Request& req, httplib::Response& res)
{
    string userEmail = req.get_param_value("email");
    if (userEmail.empty())
    {
        res.status = 400;
        res.set_content("[]", "application/json");
        return;
    }

    const string filepath = "data/orders.csv";
    lock_guard<mutex> lock(csvMutex);
    ifstream file(filepath);
    if (!file.is_open())
    {
        res.set_content("[]", "application/json");
        return;
    }

    string line;
    getline(file, line); // Skip header

    string jsonArray = "[";
    bool first = true;

    string targetEmail = userEmail;
    transform(targetEmail.begin(), targetEmail.end(), targetEmail.begin(), ::tolower);

    while (getline(file, line))
    {
        if (line.empty()) continue;
        vector<string> fields = parseCsvLine(line);
        if (fields.size() < 8) continue;

        string fullName = unquoteField(fields[0]);
        string phone = unquoteField(fields[1]);
        string address = unquoteField(fields[2]);
        string city = unquoteField(fields[3]);
        string notes = unquoteField(fields[4]);
        string paymentMethod = unquoteField(fields[5]);
        string date = unquoteField(fields[6]);
        string email = unquoteField(fields[7]);

        string lowerEmail = email;
        transform(lowerEmail.begin(), lowerEmail.end(), lowerEmail.begin(), ::tolower);

        if (lowerEmail == targetEmail)
        {
            if (!first) jsonArray += ",";
            first = false;

            jsonArray += "{";
            jsonArray += "\"fullName\":\"" + fullName + "\",";
            jsonArray += "\"phone\":\"" + phone + "\",";
            jsonArray += "\"address\":\"" + address + "\",";
            jsonArray += "\"city\":\"" + city + "\",";
            jsonArray += "\"notes\":\"" + notes + "\",";
            jsonArray += "\"paymentMethod\":\"" + paymentMethod + "\",";
            jsonArray += "\"date\":\"" + date + "\",";
            jsonArray += "\"email\":\"" + email + "\"";
            jsonArray += "}";
        }
    }
    file.close();

    jsonArray += "]";
    res.status = 200;
    res.set_content(jsonArray, "application/json");
});

    // ========================================================
    // POST SUPPORT QUERY API (SAVES FEEDBACK TO support.csv)
    // ========================================================
    svr.Post("/api/support/submit", [](const httplib::Request& req, httplib::Response& res)
    {
        const string supportFilePath = "data/support.csv";

        if (req.body.empty())
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Empty request body"})", "application/json");
            return;
        }

        string topic = getJsonField(req.body, "topic");
        string name = getJsonField(req.body, "name");
        string email = getJsonField(req.body, "email");
        string query = getJsonField(req.body, "query");
        string date = getJsonField(req.body, "date");

        if (name.empty() || email.empty() || query.empty())
        {
            res.status = 400;
            res.set_content(R"({"status":"error","message":"Missing required fields"})", "application/json");
            return;
        }

        lock_guard<mutex> lock(csvMutex);
        
        ifstream checkFile(supportFilePath);
        bool isEmpty = !checkFile.is_open() || checkFile.peek() == ifstream::traits_type::eof();
        checkFile.close();

        ofstream outFile(supportFilePath, ios::out | ios::app);
        if (!outFile.is_open())
        {
            res.status = 500;
            res.set_content(R"({"status":"error","message":"Cannot open support.csv for writing"})", "application/json");
            return;
        }

        if (isEmpty)
        {
            outFile << "topic,name,email,query,date\n";
        }

        stringstream csvLine;
        csvLine << "\"" << topic << "\","
                << "\"" << name << "\","
                << "\"" << email << "\","
                << "\"" << query << "\","
                << "\"" << date << "\"\n";

        outFile << csvLine.str();
        outFile.close();

        res.status = 200;
        res.set_content(R"({"status":"success","message":"Query saved successfully to support.csv"})", "application/json");
    });

    // ========================================================
    // SERVER START
    // ========================================================
    const char* portEnv = getenv("PORT");
    int port = portEnv ? stoi(portEnv) : 8080;
    cout << endl;
    cout << "========================================" << endl;
    cout << "       THE BREW BUDDIES SERVER" << endl;
    cout << "========================================" << endl;
    cout << "Server running on port: " << port << endl;
    cout << "Open: http://localhost:" << port << endl;
    cout << "========================================" << endl;
    cout << endl;
    if (!svr.listen("0.0.0.0", port))
    {
        cerr << "[ERROR] Failed to start server" << endl;
        return 1;
    }
    return 0;
}