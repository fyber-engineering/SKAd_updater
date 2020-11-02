# SKAd Updater 

SKAdNetwork IDs Updater

## Install

##### Using Homebrew:
    brew tap fyber-engineering/skad_updater
    brew install skad_updater

## Use

### Synopsis

    skad_updater ( (--help | -h) | (--show_networks) | --plist_path plist-file-path (--network_list <comma-separated-network-names> | --pod_path <pod-file-path>) [--dry_run] )

### Description
 Pull the most up-to-date SKAdNetwork IDs from https://github.com/fyber-engineering/SKAdNetworks and updates the info.plist appropriately.

 The list of required networks can be requested in two ways:

1.   Explicitly:
     1.  Asking for a list of supported ad network names with the `--show_networks` flag.
     1.  Passing the `[--network_list <network-name-list>]` parameter where <network-name-list> is a comma separated list of network names.
1.   Automatically deriving the required networks from a `pod file`, by using the `[ --pod_path <pod-file-path> ]` parameter where `<pod-file-path>` is the path to the pod file.

#### Parameters:

| Parameter | Value  | Description  |
| :- | :-: | :-: |
| `--plist_path` | \<plist-file-path\> | The plist file path. |
| `--network_list` | \<comma-separated-network-names\> | Only if no pod_path. Request for a specific list of networks to update. The argument is a comma separated list of network names. |
| `--pod_path` | \<pod-file-path\> | network_list if provided will be ignored. Update all the networks found in the pod file.  The argument is the path to the pod file. |
| **Optional Parameters** ||
| `--dry_run` | | Perform a dry-run. Prints out the new `plist` file instead of overwriting.|
| `--show_networks` | | Show the list of supported network names.| 
| `--help, -h` | | Give a help message and exit. |

#### Examples
     skad_updater --help 
     
     skad_updater --show_networks
     
     skad_updater --plist_path <Path to plist> --pod_path <Path to Pod File>

     skad_updater --plist_path <Path to plist> --pod_path <Path to Pod File> --dry_run

     skad_updater --plist_path <Path to plist> --network_list <CSV network list>

     skad_updater --plist_path <Path to plist> --network_list <CSV network list> --dry_run

### Backups

Current/Previous info.plist will be backed up to info.plist.bak.X in the same directory in case the plist is modified, where X is the number of backup.

### Reformatting the `plist` file

After new networks were added, the `plist` file will be formatted according to the standard XML indentation formatting. 
The xcode PList indentaion formatting might differ from the standard XML's. 
*Don't worry, it works the same.*

Making a modification to the `plist` inside xcode will reformat it back to the xcode standard, Which might result in weird pull-request diffs.

If this is an issue for you, you can run this after the `skad_updater` to reformat according to the xcode standard:

```
plutil -convert xml1 <path_to_plist_file>
```

### Debugging
#### Mock service
* Running
```
    tests/servermock/run_mock_server.sh
```
* Modifying response
```
    curl -X POST "localhost:5000/set_data" -b '{ "My_Network": ["SK_ADNETWORK_ID1","SK_ADNETWORK_ID2"]}'
```

#### Change the service endpoint
Assuming that there's a running service on `localhost:5000` :
 
    export FYBER_SKAD_NETWORKS_SERVER_HOST='http://localhost:5000/'
    
#### Debug logs
* Enable
```
export FYBER_SKAD_DEBUG_LOG=''
```
* Disable
```
unset FYBER_SKAD_DEBUG_LOG
```

## Build

All the commands must be run from the repository's base folder.
 
####  Prerequisites

* curl
* cmake 3.18
* python 3.8
* clang-format

### Automatic
```
./clean
./build
```
### Manual
```
./clean

cmake -S . -B build
cmake --build build --target format
cmake --build build --target fix-format
cmake --build build --target skad_updater
```
For more information about formatting see [Format.cmake](https://github.com/TheLartians/Format.cmake/blob/master/README.md)

##### Running Tests
###### Prerequisites
1. make sure `curl` is installed.
1. Make sure you have Python 3.8 installed.
1. Create a [virtual env](https://packaging.python.org/guides/installing-using-pip-and-virtual-environments/) under `tests/servermock` and run `pip install -r requirements.txt`.

###### To run the tests:
```
cmake --build build --target tests_run

cd build/tests/
./tests_run
```

### Package
Generates a `tar.gz` file in the `build` directory.  
If `shasum` is present in the system - the valid homebrew formula `skad_undater.rb` file will also be generated.  
```
cmake --build build --target package
```

## Author
Fyber GmbH ( support@fyber.com )
