#include <iostream>
#include <exiv2/exiv2.hpp>
#include <filesystem>

namespace fs = std::filesystem;
using namespace std;

double google_gps(const Exiv2::Value &value)
{
    double degrees = value.toRational(0).first / (double)value.toRational(0).second;
    double minutes = value.toRational(1).first / (double)value.toRational(1).second;
    double seconds = value.toRational(2).first / (double)value.toRational(2).second;

    return degrees + (minutes / 60.0) + (seconds / 3600.0);
}

void print_metadata(Exiv2::Image::UniquePtr &photo)
{
    photo->readMetadata();

    Exiv2::ExifData &e_data = photo->exifData();
    Exiv2::IptcData &ip_data = photo->iptcData();

    if (e_data.empty() && ip_data.empty())
    {
        cout << "   Image has no metadata or failed to read metadata!" << endl;
        return;
    }

    cout << "   Entries: " << endl;
    cout << "       Exiv: " << e_data.count() << " entries" << endl;
    cout << "       IPTC: " << ip_data.count() << " entries" << endl;

    cout << "   --- Exiv metadata ---" << endl;
    for (auto &i : e_data)
    {
        cout << "       " << i.key() << " = " << i.value() << endl;
    }

    auto lati = e_data["Exif.GPSInfo.GPSLatitude"];
    auto lati_ref = e_data["Exif.GPSInfo.GPSLatitudeRef"];
    auto longi = e_data["Exif.GPSInfo.GPSLongitude"];
    auto longi_ref = e_data["Exif.GPSInfo.GPSLongitudeRef"];

    if (!lati.count() || !longi.count())
    {
        std::cout << "       No GPS data found!" << endl;
    }
    else
    {
        double g_lati = google_gps(lati.value());
        double g_longi = google_gps(longi.value());

        if (lati_ref.toString() == "S")
            g_lati = -g_lati;
        if (longi_ref.toString() == "W")
            g_longi = -g_longi;

        cout << "       Google Maps GPS: " << g_lati << ", " << g_longi << endl;
    }

    cout << endl;
    cout << "   --- IPTC metadata ---" << endl;
    for (auto &i : ip_data)
    {
        cout << "       " << i.key() << " = " << i.value() << endl;
    }
}

int checking_images(fs::path dir)
{
    std::cout << "Checking for images:... " << dir << endl
              << endl;

    vector<string> extensions = {".jpg", ".jpeg", ".png", ".tiff", ".tif"};
    int image_count = 0;

    for (auto &entry : fs::directory_iterator(dir))
    {
        if (!entry.is_regular_file()) // if folder then skips
            continue;

        string image_path = entry.path().string();

        string file_ext = entry.path().extension().string();
        // make extension into lowercase
        transform(file_ext.begin(), file_ext.end(), file_ext.begin(), ::tolower);

        // if file extension matches image extensions
        if (find(extensions.begin(), extensions.end(), file_ext) != extensions.end())
        {
            cout << "\nFound: " << entry.path().filename() << endl;
            cout << "   Full path: " << entry.path() << endl;
            cout << "   Size: " << entry.file_size() << " bytes" << endl;

            // print metadata of image
            auto image = Exiv2::ImageFactory::open(image_path);
            if (!image.get())
            {
                cout << "   IMAGE FAILED TO BE OPENED!" << endl;
            }

            print_metadata(image);

            image_count++;
        }
    }

    return image_count;
}

int main(int argc, char *argv[])
{
    fs::path current_dir = fs::current_path();
    int image_num = checking_images(current_dir);

    cout << "\nNumber of images: " << image_num << endl;

    cout << "Press Enter Key To Exit...";
    cin.get();

    return 0;
}
