#include <sstream>

void Server::handleMode(int fd, const Command& cmd)
{
    if (cmd.params.empty())
    {
        sendReply(fd, "461", "MODE :Not enough parameters");
        return;
    }

    std::string target = cmd.params[0];

    // 1. Check if target is a channel (standard IRC channels start with # or &)
    if (target[0] != '#' && target[0] != '&')
    {
        // User modes are often not required for the mandatory part, 
        // but we return 501 if someone tries to change them.
        if (target != _clients[fd].getNickname())
            sendReply(fd, "401", target + " :No such nick/channel");
        else
            sendReply(fd, "501", ":Unknown MODE flag");
        return;
    }

    // 2. Check if channel exists
    std::map<std::string, Channel>::iterator it = _channels.find(target);
    if (it == _channels.end())
    {
        sendReply(fd, "403", target + " :No such channel");
        return;
    }

    Channel& chan = it->second;

    // 3. If only channel name is provided, return current modes (RPL_CHANNELMODEIS 324)
    if (cmd.params.size() == 1)
    {
        std::string modes = "+";
        if (chan.isInviteOnly()) modes += "i";
        if (chan.isTopicRestricted()) modes += "t";
        if (chan.hasKey()) modes += "k";
        if (chan.getLimit() > 0) modes += "l";
        
        sendReply(fd, "324", target + " " + modes);
        return;
    }

    // 4. Permission check: Only operators can change modes
    if (!chan.isOperator(fd))
    {
        sendReply(fd, "482", target + " :You're not channel operator");
        return;
    }

    std::string modestring = cmd.params[1];
    std::vector<std::string> modeParams;
    if (cmd.params.size() > 2)
    {
        // Collect all extra params (for o, k, l)
        for (size_t i = 2; i < cmd.params.size(); ++i)
            modeParams.push_back(cmd.params[i]);
    }

    bool adding = true; // true for '+', false for '-'
    size_t paramIdx = 0;
    std::string appliedModes = "";
    std::string appliedParams = "";

    for (size_t i = 0; i < modestring.size(); ++i)
    {
        char c = modestring[i];
        if (c == '+') { adding = true; appliedModes += "+"; continue; }
        if (c == '-') { adding = false; appliedModes += "-"; continue; }

        if (c == 'i')
        {
            chan.setInviteOnly(adding);
            appliedModes += "i";
        }
        else if (c == 't')
        {
            chan.setTopicRestricted(adding);
            appliedModes += "t";
        }
        else if (c == 'k')
        {
            if (adding && paramIdx < modeParams.size())
            {
                chan.setKey(modeParams[paramIdx]);
                appliedModes += "k";
                appliedParams += " " + modeParams[paramIdx];
                paramIdx++;
            }
            else if (!adding)
            {
                chan.setKey("");
                appliedModes += "k";
            }
        }
        else if (c == 'l')
        {
            if (adding && paramIdx < modeParams.size())
            {
                int limit = std::atoi(modeParams[paramIdx].c_str());
                if (limit > 0)
                {
                    chan.setLimit(static_cast<size_t>(limit));
                    appliedModes += "l";
                    appliedParams += " " + modeParams[paramIdx];
                }
                paramIdx++;
            }
            else if (!adding)
            {
                chan.setLimit(0);
                appliedModes += "l";
            }
        }
        else if (c == 'o')
        {
            if (paramIdx < modeParams.size())
            {
                Client* targetClient = getClientByNick(modeParams[paramIdx]);
                if (!targetClient || !chan.isClientInChannel(targetClient->getFd()))
                {
                    sendReply(fd, "441", modeParams[paramIdx] + " " + target + " :They aren't on that channel");
                }
                else
                {
                    if (adding)
                        chan.addOperator(targetClient);
                    else
                        chan.removeOperator(targetClient->getFd());
                    
                    appliedModes += "o";
                    appliedParams += " " + modeParams[paramIdx];
                }
                paramIdx++;
            }
            else
                sendReply(fd, "461", "MODE :Not enough parameters");
        }
        else
        {
            // ERR_UNKNOWNMODE (472)
            std::string err; err += c;
            sendReply(fd, "472", err + " :is unknown mode char to me");
        }
    }

    // 5. Broadcast change to the channel if modes were actually changed
    if (appliedModes.size() > 1 || (appliedModes.size() == 1 && appliedModes[0] != '+' && appliedModes[0] != '-'))
    {
        std::string notify = ":" + _clients[fd].getNickname() + "!" + _clients[fd].getUsername() 
                           + "@" + _clients[fd].getHostname() + " MODE " + target + " " 
                           + appliedModes + appliedParams + "\r\n";
        
        const std::map<int, Client*>& clients = chan.getClients();
        for (std::map<int, Client*>::const_iterator cit = clients.begin(); cit != clients.end(); ++cit)
        {
            cit->second->getOutputBuffer() += notify;
            notifyPollout(cit->first);
        }
    }
}