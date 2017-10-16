# User login init file
# Version 0.8.3, March 2016
# Add init commands to this file to be run on login.

[Section login]

# Set up graphical desktop
start service Xserver.xorg
source .xprofile

# Load preferences
source userprefs.list

# Mount filesystem
mount -f /afs/${USER} ~/

# Decrypt files
ecryptfs -k ~/.homekey ~/

# Display welcome
echo Welcome!
xwelcome

alias foobar barfood
alias ls ls -i
alias cat dog

end
