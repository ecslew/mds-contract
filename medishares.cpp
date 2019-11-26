#include <hbtcoop.hpp>
#include <eosiolib/time.hpp>

using namespace eosio;

uint64_t year_second = 365*24*3600;

void hbtcoop::add(const account_name initiator, const std::string name, const checksum256 item_digest, const account_name receiver, const extended_asset& min_fund, const extended_asset& max_fund, const extended_asset& target_fund, const uint64_t deadline)
{
	eosio_assert( name.size() <= 256, "the name of item has more than 256 bytes");
	eosio_assert( is_account(receiver), "receiver account does not exist");
	eosio_assert( min_fund.is_valid() && max_fund.is_valid() && target_fund.is_valid(), "invalid fund" );
	eosio_assert( min_fund.amount > 0 && min_fund.amount <= max_fund.amount && max_fund.amount <= target_fund.amount, "invalid fund quantity" );
	eosio_assert( min_fund.contract == max_fund.contract && max_fund.contract == target_fund.contract, "inconsistent asset");
	eosio_assert( is_account(min_fund.contract), "invalid asset");
	eosio_assert( deadline > now() && deadline - now() < year_second, "invalid deadline");

	require_auth(initiator);

	auto glb = global.begin();
    if(glb == global.end()){
        glb = global.emplace(_self, [&](auto& gl){
            gl.total_item = 1;
            gl.completed_item = 0;
            gl.ongoing_item = 0;
			gl.auditor = _self;
        }); 
    }else{
        global.modify(glb, 0, [&](auto& gl){
        	gl.total_item = gl.total_item + 1;
		}); 
    }
	
	items_table items(_self, initiator);
	auto ite = items.begin();
	while(ite != items.end()){
		if(ite->name == name)
			eosio_assert(false, "the item name aready exist");
		ite++;
	}
	
    items.emplace(initiator, [&](auto& it){
        it.id = glb->total_item;
        it.name = name;
		it.item_digest = item_digest;
        it.initiator = initiator;
        it.receiver = receiver;
        it.min_fund = min_fund;
		it.max_fund = max_fund;
		it.target_fund = target_fund;
        it.start = now();
		it.deadline = deadline;
		it.status = 0;
    }); 

}
    
void hbtcoop::modify(const account_name initiator, const uint64_t id, const std::string name, const checksum256 item_digest, const account_name receiver, const extended_asset& min_fund, const extended_asset& max_fund, const extended_asset& target_fund, const uint64_t deadline)
{
    eosio_assert( name.size() <= 256, "the name of item has more than 256 bytes");
    eosio_assert( is_account(receiver), "receiver account does not exist");    
	eosio_assert( min_fund.is_valid() && max_fund.is_valid() && target_fund.is_valid(), "invalid fund" );    
	eosio_assert( min_fund.amount > 0 && min_fund.amount <= max_fund.amount && max_fund.amount <= target_fund.amount, "invalid fund quantity" );
    eosio_assert( min_fund.contract == max_fund.contract && max_fund.contract == target_fund.contract, "inconsistent asset");
    eosio_assert( is_account(min_fund.contract), "invalid asset");
    eosio_assert( deadline > now() && deadline - now() < year_second, "invalid deadline");

	items_table items(_self, initiator);
	auto ite = items.begin();
    while(ite != items.end() && ite->id != id)
        ite++;
    eosio_assert(ite != items.end(), "the item not exist");
	eosio_assert(ite->status < 2, "item can not be modified after audited");

	require_auth(ite->initiator);
	
    items.modify(ite, 0, [&](auto& it){
        it.name = name;
        it.item_digest = item_digest;
        it.receiver = receiver;
        it.min_fund = min_fund;
        it.max_fund = max_fund;
        it.target_fund = target_fund;
        it.deadline = deadline;
		it.status = 0;
    });
}

void hbtcoop::erase(const account_name initiator, const std::string name)
{
	items_table items(_self, initiator);
    auto ite = items.begin();
    while(ite != items.end() && ite->name != name)
        ite++;
    eosio_assert(ite != items.end(), "the item not exist");
    eosio_assert(ite->status < 2, "item cannot be deleted after audited");

	require_auth(ite->initiator);    
    items.erase(ite);
}

void hbtcoop::audit(const account_name initiator, const std::string name, const uint8_t flag)
{
	items_table items(_self, initiator);
    auto ite = items.begin();
    while(ite != items.end() && ite->name != name)
        ite++;
    eosio_assert(ite != items.end(), "the item not exist");
    eosio_assert(ite->status < 2, "item has been audited aready");
    eosio_assert(ite->deadline > now(), "item expired");

	auto glb = global.begin();
	eosio_assert(glb != global.end(), "the global table does not exist");
    require_auth(glb->auditor);

	if(flag == 2){
		items.modify(ite, 0, [&](auto& it){
			it.start = now();
			it.status = 2;
		});

        global.modify(glb, 0, [&](auto& gl){
            gl.ongoing_item = gl.ongoing_item + 1;
        });
	}else if(flag == 1){
		items.modify(ite, 0, [&](auto& it){
            it.status = 1;
        });
	}else if(flag == 0){
		items.erase(ite);
	}
}

void hbtcoop::setauditor(const account_name auditor)
{
	eosio_assert( is_account(auditor), "auditor account does not exist");
	auto glb = global.begin();
    eosio_assert(glb != global.end(), "the global table does not exist");
    require_auth(_self);

	global.modify(glb, 0, [&](auto& gl){
        gl.auditor = auditor;
    });
}
